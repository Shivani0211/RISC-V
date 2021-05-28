module top;

real a_real, b_real = 0.0;
real c_real;
logic [63:0] a,b;
wire [63:0] c;

 dut       I (.a(a), .b(b), .c(c));

always@(a_real) a <= $realtobits(a_real);
always@(b_real) b <= $realtobits(b_real);
always@(c) c_real <= $bitstoreal(c);

 initial
 begin
   a_real <= 0.01;
   b_real <= 0.02;
   #5;
   a_real <= 0.02;
   b_real <= 0.03;
   #10;
   $finish;
 end

 always@(c)
 begin
	#5
    $display("Input a = %b, b = %b, Output c = %b\n", a,b,c);
	$display("Input a_real = %f, b_real = %f, Output c_real = %f\n", a_real,b_real,c_real);
    #5;
    $display("Input a = %b, b = %b, Output c = %b\n", a,b,c);
	$display("Input a_real = %f, b_real = %f, Output c_real = %f\n", a_real,b_real,c_real);
    
 end


endmodule

