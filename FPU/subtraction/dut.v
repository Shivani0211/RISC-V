module dut (input [63:0] a,b,
             output [63:0]  c);
real a_real, b_real, c_real;

assign c = $realtobits(c_real);
always@(a) a_real = $bitstoreal(a);
always@(b) b_real = $bitstoreal(b);

initial
	$dut_c(a_real, b_real, c_real);

endmodule

