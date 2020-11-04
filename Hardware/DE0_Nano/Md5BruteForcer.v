module Md5BruteForcer(
	input wire clk,
	input wire reset,
	input wire hasReceived,
	input wire [31:0] dataIn,
	output reg hasMatched = 0,
	output reg [31:0] dataOut = 0,
	output reg [127:0] text = 0,
	output reg resetGenerator = 0
);

reg matchFound = 0;

wire [31:0] a, b, c, d;
//reg [31:0] expectedA = 'hffffffff, expectedB = 0, expectedC = 0, expectedD = 0;
//reg [31:0] expectedA = 'h4e905851, expectedB = 'hd414de4d, expectedC = 'h2dba0c2c, expectedD = 'h75c54aac;
reg [31:0] expectedA = 'h2971bc83, expectedB = 'h9b41f6a4, expectedC = 'h955620c0, expectedD = 'h9067fbfd;
reg [63:0] count;
reg [7:0] min = 'h61, max = 'h7a;
//reg [7:0] min = 'h41, max = 'h7a;
wire [511:0] chunk;

reg [127:0] textBuffer;
reg [7:0] txttmp = 'h00;
reg [7:0] carry = 'h00;


Md5PrintableChunkGenerator g(
	.clk(clk), 
	.reset(resetGenerator),
	.chunk(chunk),
	.min(min),
	.max(max)	
);

/*
Md5ChunkGenerator g(
	.clk(clk), 
	.reset(resetGenerator),
	.chunk(chunk)
);
*/

Md5Core md5 (
	.clk(clk), 
	.wb(chunk), 
	.a0('h67452301), 
	.b0('hefcdab89), 
	.c0('h98badcfe), 
	.d0('h10325476), 
	.a64(a), 
	.b64(b), 
	.c64(c), 
	.d64(d)
);

reg reset2 = 0;

always @(posedge clk or posedge reset2)
	begin
		if (reset2)
			begin
				hasMatched <= 0;
				matchFound <= 0;
				hasMatched <= 0;
				text <= 0;
            count <= 0;
			end
		else
			begin
				if (!matchFound)
					begin

					textBuffer <= chunk[127:0];

					//initial is an offset of 0x0B
					// 1
					txttmp = chunk[7:0];
					if((txttmp - 'h0B) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - 'h0B;
							textBuffer[7:0] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ('h0B - (txttmp - min)) + 'h01;
							textBuffer[7:0] <= txttmp;
							carry = 'h01;
						end
					//the next is 0x02, unless there is a carry and then it is + 1
					// 2
					txttmp = chunk[15:8];
					if((txttmp - 'h02) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - 'h02 - carry;
							textBuffer[15:8] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ('h02 - (txttmp - min)) + 'h01 - carry;
							textBuffer[15:8] <= txttmp;
							carry = 'h01;
						end
					//everything should just be a carry after this?
					// 3
					txttmp = chunk[23:16];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[23:16] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[23:16] <= txttmp;
							carry = 'h01;
						end
					// 4
					txttmp = chunk[31:24];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[31:24] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[31:24] <= txttmp;
							carry = 'h01;
						end
					// 5
					txttmp = chunk[39:32];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[39:32] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[39:32] <= txttmp;
							carry = 'h01;
						end
					// 6
					txttmp = chunk[47:40];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[47:40] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[47:40] <= txttmp;
							carry = 'h01;
						end
					// 7
					txttmp = chunk[55:48];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[55:48] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[55:48] <= txttmp;
							carry = 'h01;
						end
					// 8
					txttmp = chunk[63:56];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[63:56] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[63:56] <= txttmp;
							carry = 'h01;
						end
					// 9
					txttmp = chunk[71:64];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[71:64] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[71:64] <= txttmp;
							carry = 'h01;
						end
					// 10
					txttmp = chunk[79:72];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[79:72] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[79:72] <= txttmp;
							carry = 'h01;
						end
					// 11
					txttmp = chunk[87:80];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[87:80] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[87:80] <= txttmp;
							carry = 'h01;
						end
					// 12
					txttmp = chunk[95:88];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[95:88] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[95:88] <= txttmp;
							carry = 'h01;
						end
					// 13
					txttmp = chunk[103:96];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[103:96] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[103:96] <= txttmp;
							carry = 'h01;
						end
					// 14
					txttmp = chunk[111:104];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[111:104] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[111:104] <= txttmp;
							carry = 'h01;
						end
					// 15
					txttmp = chunk[119:112];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[119:112] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[119:112] <= txttmp;
							carry = 'h01;
						end
					// 16
					txttmp = chunk[127:120];
					if((txttmp - carry) >= min && txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = txttmp - carry;
							textBuffer[127:120] <= txttmp;
							carry = 'h00;
						end
					else if(txttmp != 'h00 && txttmp != 'h80)
						begin
							txttmp = max - ((txttmp - min)) + 'h01 - carry;
							textBuffer[127:120] <= txttmp;
							carry = 'h01;
						end						
/**/


					text <= textBuffer;

					
/**
					textBuffer[0] <= chunk[127:0];
						textBuffer[1] <= textBuffer[0];
						textBuffer[2] <= textBuffer[1];
						textBuffer[3] <= textBuffer[2];
						textBuffer[4] <= textBuffer[3];
						textBuffer[5] <= textBuffer[4];
						textBuffer[6] <= textBuffer[5];
						textBuffer[7] <= textBuffer[6];
						textBuffer[8] <= textBuffer[7];
						textBuffer[9] <= textBuffer[8];
						textBuffer[10] <= textBuffer[9];
						textBuffer[11] <= textBuffer[10];
						textBuffer[12] <= textBuffer[11];
						textBuffer[13] <= textBuffer[12];
						textBuffer[14] <= textBuffer[13];
						textBuffer[15] <= textBuffer[14];
						textBuffer[16] <= textBuffer[15];
						textBuffer[17] <= textBuffer[16];
						textBuffer[18] <= textBuffer[17];
						textBuffer[19] <= textBuffer[18];
						textBuffer[20] <= textBuffer[19];
						textBuffer[21] <= textBuffer[20];
						textBuffer[22] <= textBuffer[21];
						textBuffer[23] <= textBuffer[22];
						textBuffer[24] <= textBuffer[23];
						textBuffer[25] <= textBuffer[24];
						textBuffer[26] <= textBuffer[25];
						textBuffer[27] <= textBuffer[26];
						textBuffer[28] <= textBuffer[27];
						textBuffer[29] <= textBuffer[28];
						textBuffer[30] <= textBuffer[29];
						textBuffer[31] <= textBuffer[30];
						textBuffer[32] <= textBuffer[31];
						textBuffer[33] <= textBuffer[32];
						textBuffer[34] <= textBuffer[33];
						textBuffer[35] <= textBuffer[34];
						textBuffer[36] <= textBuffer[35];
						textBuffer[37] <= textBuffer[36];
						textBuffer[38] <= textBuffer[37];
						textBuffer[39] <= textBuffer[38];
						textBuffer[40] <= textBuffer[39];
						textBuffer[41] <= textBuffer[40];
						textBuffer[42] <= textBuffer[41];
						textBuffer[43] <= textBuffer[42];
						textBuffer[44] <= textBuffer[43];
						textBuffer[45] <= textBuffer[44];
						textBuffer[46] <= textBuffer[45];
						textBuffer[47] <= textBuffer[46];
						textBuffer[48] <= textBuffer[47];
						textBuffer[49] <= textBuffer[48];
						textBuffer[50] <= textBuffer[49];
						textBuffer[51] <= textBuffer[50];
						textBuffer[52] <= textBuffer[51];
						textBuffer[53] <= textBuffer[52];
						textBuffer[54] <= textBuffer[53];
						textBuffer[55] <= textBuffer[54];
						textBuffer[56] <= textBuffer[55];
						textBuffer[57] <= textBuffer[56];
						textBuffer[58] <= textBuffer[57];
						textBuffer[59] <= textBuffer[58];
						textBuffer[60] <= textBuffer[59];
						textBuffer[61] <= textBuffer[60];
						textBuffer[62] <= textBuffer[61];
						textBuffer[63] <= textBuffer[62];
						text <= textBuffer[63];
**/
						if (a == expectedA &&
							b == expectedB &&
							c == expectedC &&
							d == expectedD)
							begin
								matchFound <= 1;
								hasMatched <= 1;
							end

                  count <= count + 1;
					end
			end
	end

	
`define Controller_Waiting 			'h1
`define Controller_SetExpectedA		'h2
`define Controller_SetExpectedB		'h3
`define Controller_SetExpectedC		'h4
`define Controller_SetExpectedD		'h5
`define Controller_SetRange			'h6

`define Command_NoOp                'h00000000

`define Command_ResetGenerator 		'h52300000
`define Command_StartGenerator 		'h52300001

`define Command_SetExpectedA		 	'h52301000
`define Command_SetExpectedB		 	'h52301001
`define Command_SetExpectedC		 	'h52301002
`define Command_SetExpectedD		 	'h52301003

`define Command_SetRange				'h52302000

`define Command_GetCountLow         'h52303000
`define Command_GetCountHigh        'h52303001

`define Command_GetTextChar1			'h44000001
`define Command_GetTextChar2			'h44000002
`define Command_GetTextChar3			'h44000003
`define Command_GetTextChar4			'h44000004



reg [7:0] controllerState = `Controller_Waiting;

always @(posedge hasReceived)
	begin
		case (controllerState)
			`Controller_Waiting:
				begin
					case (dataIn)
						`Command_NoOp:
							begin
								dataOut <= 0;
							end
						`Command_ResetGenerator: 
							begin
								resetGenerator <= 1;
								reset2 <= 1;
								dataOut <= 0;
							end
						`Command_StartGenerator:
							begin
								resetGenerator <= 0;
								reset2 <= 0;
							end
						`Command_SetExpectedA:
							begin
								controllerState <= `Controller_SetExpectedA;
							end
						`Command_SetExpectedB:
							begin
								controllerState <= `Controller_SetExpectedB;
							end
						`Command_SetExpectedC:
							begin
								controllerState <= `Controller_SetExpectedC;
							end
						`Command_SetExpectedD:
							begin
								controllerState <= `Controller_SetExpectedD;
							end
						`Command_SetRange:
							begin
								controllerState <= `Controller_SetRange;
							end
                  `Command_GetCountLow:
							begin
								dataOut <= count[31:0];
							end
                  `Command_GetCountHigh:
							begin
								dataOut <= count[63:32];
							end
						`Command_SetExpectedA: controllerState <= `Controller_SetExpectedA;
						`Command_SetExpectedB: controllerState <= `Controller_SetExpectedB;
						`Command_SetExpectedC: controllerState <= `Controller_SetExpectedC;
						`Command_SetExpectedD: controllerState <= `Controller_SetExpectedD;
						`Command_SetRange: controllerState <= `Controller_SetRange;
                  `Command_GetCountLow: dataOut <= count[31:0];
                  `Command_GetCountHigh: dataOut <= count[63:32];
						`Command_GetTextChar1:
							begin
									dataOut <= text[31:0];
							end
						`Command_GetTextChar2:
							begin
									dataOut <= text[63:32];
							end
						`Command_GetTextChar3:
							begin
									dataOut <= text[95:64];
							end
						`Command_GetTextChar4:
							begin
									dataOut <= text[127:96];
							end
					endcase					
				end
			`Controller_SetExpectedA:
				begin
					expectedA <= dataIn;
					controllerState <= `Controller_Waiting;
				end
			`Controller_SetExpectedB:
				begin
					expectedB <= dataIn;
					controllerState <= `Controller_Waiting;
				end
			`Controller_SetExpectedC:
				begin
					expectedC <= dataIn;
					controllerState <= `Controller_Waiting;
				end
			`Controller_SetExpectedD:
				begin
					expectedD <= dataIn;
					controllerState <= `Controller_Waiting;
				end
			`Controller_SetRange:
				begin
					min <= dataIn[7:0];
					max <= dataIn[15:8];
					//min <= 8'h61;
					//max <= 8'h7a;
					controllerState <= `Controller_Waiting;
				end
		endcase	
	end

endmodule