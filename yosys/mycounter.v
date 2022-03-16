module mycounter(input clk,output reg[2:0] cnt);
    initial cnt = 0;
    always@(posedge clk) begin
        if (cnt==3)
            cnt <= 0;
        else
            cnt <= cnt + 1;
    end
    `ifdef FORMAL
        always @(posedge clk) begin
        assert(cnt < 2);
        end
    `endif

endmodule