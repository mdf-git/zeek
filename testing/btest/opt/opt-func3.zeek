# @TEST-EXEC: zeek -b -O ZAM --optimize-func='zeek_init' --optimize-func='my_test' %INPUT >output
# @TEST-EXEC: btest-diff output

# Tests that we can selectively a bunch of functions (event handlers),
# and also call out an additional function.

function my_test()
	{
	print "Me and my buds should be ZAM code!";
	}

event zeek_init()
	{
	my_test();
	print my_test;
	print zeek_init;
	}
