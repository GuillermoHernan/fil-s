using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;

namespace testRunner
{
    /// <summary>
    /// Test runner: program which run tests whi involved compiling and executing a
    /// FIL-S program, launching both the compiler and external program as external 
    /// processes.
    /// </summary>
    class TestRunner
    {
        static void Main(string[] args)
        {            
            string compilerPath = findCompiler();
            string testsPath = findTests();

            runTests(testsPath, compilerPath);
        }


        /// <summary>
        /// Executes the tests found in test directory.
        /// </summary>
        /// <param name="testsPath"></param>
        /// <param name="compilerPath"></param>
        private static void runTests(string testsPath, string compilerPath)
        {
            var paths = Directory.GetDirectories(testsPath);
            int successCount = 0;
            int failCount = 0;

            foreach (var path in paths)
            {
                if (!path.StartsWith("."))
                {
                    if (runTest(path, compilerPath))
                        ++successCount;
                    else
                        ++failCount;
                }
            }

            string message = String.Format("Finished. Passed: {0} Failed: {1}", successCount, failCount);
            Console.Out.WriteLine(message);
        }

        /// <summary>
        /// Executes a single test.
        /// </summary>
        /// <param name="path"></param>
        /// <param name="compilerPath"></param>
        private static bool runTest(string path, string compilerPath)
        {
            if (compileTest(path, compilerPath))
                return launchTestedProgram(path);
            else
                return false;
        }

        /// <summary>
        /// Compiles a test program.
        /// </summary>
        /// <param name="path"></param>
        /// <param name="compilerPath"></param>
        /// <returns></returns>
        private static bool compileTest(string path, string compilerPath)
        {
            var result = launchProcess(compilerPath, "\"" + path + "\"");

            if (!result.launched)
            {
                Console.Out.WriteLine("Failed to launch compiler!");
                return false;
            }
            else if (result.code == 0)
            {
                Console.Out.WriteLine("PASSED: " + path);
                return true;
            }
            else
            {
                Console.Out.WriteLine("FAILED: " + path);
                Console.Out.WriteLine(result.stdout);
                return false;
            }
        }

        /// <summary>
        /// Launches an external process, waits for it to finish, and returns its
        /// results.
        /// </summary>
        /// <param name="exePath"></param>
        /// <param name="args"></param>
        /// <returns></returns>
        private static ProcessResult launchProcess(string exePath, string args)
        {
            var result = new ProcessResult();
            try
            {
                var pi = new ProcessStartInfo(exePath, args);

                pi.RedirectStandardOutput = true;

                var proc = Process.Start(pi);

                result.stdout = proc.StandardOutput.ReadToEnd();
                proc.WaitForExit();

                result.launched = true;
                result.code = proc.ExitCode;
            }
            catch (Exception ex)
            {
                Console.Out.WriteLine("Error launching process: " + ex.Message);
                result.launched = false;
            }
            return result;
        }

        /// <summary>
        /// Launch a tested program.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        private static bool launchTestedProgram(string path)
        {
            string name = Path.GetFileName(path);
            string exePath = Path.Combine(path, name + ".exe");

            var result = launchProcess(exePath, "");

            if (!result.launched)
            {
                Console.Out.WriteLine("Failed to test program!");
                return false;
            }
            else
                return checkTestOutput(result.stdout);
        }

        /// <summary>
        /// Checks a program test output.
        /// </summary>
        /// <param name="stdout"></param>
        /// <returns></returns>
        private static bool checkTestOutput(string stdout)
        {
            var lines = stdout.Split("\n".ToCharArray());

            for (int i = lines.Length -1; i >= 0; --i)
            {
                string line = lines[i].Trim();

                if (line.StartsWith ("o15="))
                {
                    if (line.Length >= 5 && line.Substring(4) == "1")
                        return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Finds the tests folder.
        /// </summary>
        /// <returns></returns>
        private static string findTests()
        {
            string myLocation = System.Reflection.Assembly.GetEntryAssembly().Location;
            var curPath = new DirectoryInfo(Path.GetDirectoryName(myLocation));

            while (curPath != null)
            {
                string testsPath = Path.Combine(curPath.FullName, "compiler_tests");

                if (Directory.Exists(testsPath))
                    return testsPath;
                else
                    curPath = curPath.Parent;
            }

            return "";
        }

        /// <summary>
        /// Looks for the compiler executable.
        /// </summary>
        /// <returns></returns>
        private static string findCompiler()
        {
            string myLocation = System.Reflection.Assembly.GetEntryAssembly().Location;
            var curPath = new DirectoryInfo(Path.GetDirectoryName(myLocation));

            while (curPath != null)
            {
                string compilerPath = Path.Combine(curPath.FullName, "bin\\filsc.exe");

                if (File.Exists(compilerPath))
                    return compilerPath;
                else
                    curPath = curPath.Parent;
            }

            return "";
        }

        /// <summary>
        /// Structure which contains process execution result information.
        /// </summary>
        private struct ProcessResult
        {
            public bool launched;
            public int code;
            public string stdout;
        }
    }
}
