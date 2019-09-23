import java.util.Optional;

public class example {
	static { System.loadLibrary("so"); }

	public static void main(String[] args)
	{
		CTest1 test1 = new CTest1();
		test1.withString("test_test_test");
		CTest2 test2 = CTest2.createCTest2(Optional.of(test1));
		CTest4 test4 = new CTest4();
		test4.setCTest2(test2);
		test4.fun(()->System.out.println("test4 fun!\n"));
	}
}
