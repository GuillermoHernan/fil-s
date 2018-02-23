actor OtherActor (p1: int = 5, p2: int = 0) {
	output o1 (int, int);
	output o2 (int);
	output o3 (int);
	
	input i3(a: int){
		//...
	}
	//...
};

actor Test{
	child : OtherActor
	child' : OtherActor
	
	//Unnamed input (not accessible by clients)
	child.o1 -> (a: int, b: int)
	{
		//...
	}
	
	//Connection to named input. The input is accessible by clients
	//Do we want this?
	input i2 (x: int) <- child.o2
	{
		//...
	}
	
	//Can also be written in this way:
	input i2 (x: int)
	{
		//...
	}
	child->o2 ->(x) {i2(x);}
	
	
	//Connect children.
	child.o3 -> child'.i3
	
	//Same thing, reverse order
	//Do we want this?
	child.i3 <- child'.o3
}

//Actor constructor parameters:
//Option 1: (not supported by current parser)
actor Test{
	child : OtherActor (1,2)

	//...
}

//Actor constructor parameters:
//Option 2: (better support by current parser)
actor Test{
	child : OtherActor = (1,2)

	//...
}

//Option 2 would only require type check modification. Option 1 would also require to modify the parser
//But, with option 1, may be the type checker would be a little simpler.


//Basics:
// * Child actor creation (only in actor body). Make a decision about parameter passing.
// * Connect to unnamed input.

//Nice to have:
// * Connection between children. Can be done with an unnamed input.
// * Check only one connection per output.
// * Connection to named input. Can be done with an unnamed input.
// * Reversed connection between children. Pure syntax sugar.