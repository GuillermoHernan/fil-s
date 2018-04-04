#pragma once

/// <summary>
/// Functions which should implement the system - specific 'C' runtime code.
/// </summary>

void system_disableInterrupts();
void system_enableInterrupts();
void system_stop();
