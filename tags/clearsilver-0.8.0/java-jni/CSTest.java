
import java.io.*;
import java.util.*;

class CSTest {

    public static void main( String [] args ) throws IOException {
	org.clearsilver.HDF hdf = new org.clearsilver.HDF();
	System.out.print("hdfptr: ");
	
	System.out.println( hdf.hdfptr );

	hdf.setValue("Foo.Bar","10");
        hdf.setValue("Foo.Baz","20");
        System.out.println( hdf.dump() );

	System.out.println( "----" );

	org.clearsilver.CS cs = new org.clearsilver.CS(hdf);
	
	String tmplstr = "Foo.Bar:<?cs var:Foo.Bar ?>\nFoo.Baz:<?cs var:Foo.Baz ?>\n";
	System.out.println(tmplstr);
	System.out.println("----");

	cs.parseStr(tmplstr);
	System.out.println(cs.render());

    }
};
