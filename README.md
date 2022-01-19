# Risc-v Simulator

Risc-v Simulator  

PS: "re" means that it has an old realization, but has failed during the past month.  

## Risc-v Simulator 1.0  
Realize a sequence cpu supporting basic instructions    

But without adding 5-stage pipeline.  

## Risc-v Simulator 1.5  
Add 5-stage pipeline to the former version.  

Modify the mem-stage to three cycles and congest EXE,ID,IF stage during the previous two cycles. 

Solve the control hazard with an array called "pcseq",   
marking down the pc seqences and compare them to the correct seqences in the MEM stage.  

Solve the data hazard by bubbling twice when tested in the ID stage.  

## Risc-v Simulator 3.0  
Improving the method of solving control hazard:  

Directly calculate the transfer address in the ID stage  

Execute the tranfer operation coded by jal(r) directly  

Predict the branch selection by a two-bit counter predictor.  

## Risc-v Simulator 4.0  
Improving the method of solving data hazard:  

Sending back the result calculated in the EXE and MEM stage to decoding stage directly  
