#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
unsigned pc;
unsigned reg[32];
unsigned memory[500000];
void read(const char*filepath);
void initialize();
int IF();
unsigned ID();
void EX();
void MEM();
void WB();
void IF_congest();
void ID_congest();
void EX_congest();
bool predict();
bool congestmat[7][5];
unsigned congestptr;
unsigned rs[]={123,43,88,159,58,178,20,175,106,40,115,94,137,105,171,50,134,186};
unsigned pn=0;
struct IF_IDReg{
    unsigned curpc,nextseqpc,ins;
}IF_IDR;
struct ID_EXReg{
    unsigned kind,imm,rs1v,rs2v,rd,func,curpc,nextseqpc;
}ID_EXR;
struct EX_MEMReg{
    unsigned rd,rdv;
    bool rdw;
    unsigned jmpaddr;
    bool jmp;
    unsigned memaddr,memdata,memlen;
    bool memvisit,meml,mems,memu;
    unsigned memrmcycs;
    unsigned nextseqpc;
}EX_MEMR;
struct MEM_WBReg{
    unsigned rd,rdv;
    bool rdw;
}MEM_WBR;
unsigned insn;
unsigned dbitspdct;
unsigned totalpdct,pdctjmp,pdctseq,pdctjmp_right,pdctseq_right;
int main(){
    char dirpath[]="D:/ComputerScience/RISC-V/testcases_for_riscv/testcases/";
    DIR *dir;
	struct dirent *ptr;
    if ((dir=opendir(dirpath)) == NULL){
		perror("Open dir error...");
        exit(1);
    }
    while ((ptr=readdir(dir)) != NULL){
        char filename[40];
        strcpy(filename,ptr->d_name);
        char*p=strchr(filename,'.');
        if(strncmp(p+1,"data",4)==0){
            char filepath[200];
            strcpy(filepath,dirpath);
            strcat(filepath,filename);
            *p='\0';
            printf("%s:\n",filename);
            initialize();
            read(filepath);
            unsigned EXcond=0;
            unsigned MEMcond=0;
            while(true){
                WB();
                MEM();
                if(!congestmat[congestptr][2]){
                    if(MEMcond==1){
                        ID_EXR.rs1v=MEM_WBR.rdv;
                    }
                    else if(MEMcond==2){
                        ID_EXR.rs2v=MEM_WBR.rdv;
                    }
                    else if(MEMcond==3){
                        ID_EXR.rs1v=MEM_WBR.rdv;
                        ID_EXR.rs2v=MEM_WBR.rdv;
                    }
                    MEMcond=0;
                }
                if(congestmat[congestptr][2]) EX_congest();
                else{
                    EX();
                    if(EXcond==1){
                        if(EX_MEMR.jmp==false){
                            IF_IDR.curpc=0;
                            IF_IDR.nextseqpc=0;
                            IF_IDR.ins=0x00000013;
                            pc=EX_MEMR.nextseqpc;
                            if(dbitspdct!=0) dbitspdct-=1;
                        }
                        else{
                            pdctjmp_right++;
                        }
                        EXcond=0;
                    }
                    else if(EXcond==2){
                        if(EX_MEMR.jmp==true){
                            IF_IDR.curpc=0;
                            IF_IDR.nextseqpc=0;
                            IF_IDR.ins=0x00000013;
                            pc=EX_MEMR.jmpaddr;
                            if(dbitspdct!=3) dbitspdct+=1;
                        }
                        else{
                            pdctseq_right++;
                        }
                        EXcond=0;
                    }
                }
                if(congestmat[congestptr][3]) ID_congest();
                else {
                    unsigned IDrs=ID();
                    switch(IDrs){
                        case 0:break;
                        case 1:EXcond=1; break;
                        case 2:EXcond=2; break;
                        case 3:break;

                        case 4:MEMcond=1;break;
                        case 5:EXcond=1;MEMcond=1;break;
                        case 6:EXcond=2;MEMcond=1;break;
                        case 7:break;

                        case 8:MEMcond=2;break;
                        case 9:EXcond=1;MEMcond=2;break;
                        case 10:EXcond=2;MEMcond=2;break;
                        case 11:MEMcond=2;break;

                        case 12:MEMcond=3;break;
                        case 13:EXcond=1;MEMcond=3;break;
                        case 14:EXcond=2;MEMcond=3;break;
                        case 15:break;
                        default:;
                    }
                }
                if(congestmat[congestptr][4]) IF_congest();
                else {if(IF()) break;}
                for(int i=0;i<5;i++) congestmat[congestptr][i]=0;
                congestptr=(congestptr+1)%7;
                insn++;
            }
            pn++;
        }
    }
    system("pause");
    return 0;
}
void initialize(){
    pc=0;
    for(int i=0;i<32;i++){
        reg[i]=0;
    }
    for(int i=0;i<500000;i++){
        memory[i]=0;
    }
    for(int k=0;k<7;k++){
        for(int i=0;i<5;i++) congestmat[k][i]=0;
    }
    insn=0;
    IF_IDR.curpc=0;IF_IDR.nextseqpc=0;IF_IDR.ins=0;
    ID_EXR.kind=0;ID_EXR.imm=0;ID_EXR.rs1v=0;ID_EXR.rs2v=0;
    ID_EXR.rd=0;ID_EXR.func=0;ID_EXR.curpc=0;ID_EXR.nextseqpc=0;
    EX_MEMR.rd=0;EX_MEMR.rdv=0;EX_MEMR.rdw=0;EX_MEMR.jmpaddr=0;
    EX_MEMR.jmp=0;EX_MEMR.memaddr=0;EX_MEMR.memdata=0;EX_MEMR.memlen=0;
    EX_MEMR.memvisit=0;EX_MEMR.meml=0;EX_MEMR.mems=0;EX_MEMR.memu=0;
    EX_MEMR.memrmcycs=0;EX_MEMR.nextseqpc=0;
    MEM_WBR.rd=0;MEM_WBR.rdv=0;MEM_WBR.rdw=0;
    dbitspdct=1;
    totalpdct=0;pdctjmp=0;pdctseq=0;pdctjmp_right=0;pdctseq_right=0;
}
void EX_congest(){}
void ID_congest(){}
void IF_congest(){}
int IF(){
    unsigned ins=memory[pc]|memory[pc+1]<<8|memory[pc+2]<<16|memory[pc+3]<<24;
    
    if(ins==0x0ff00513&&insn>15){
        printf("return:%d real:%d\n",reg[10]&0xFF,rs[pn]);
        printf("Situation of prediction:\ntotalpdct:%d  pdctjmp:%d  pdctseq:%d\npdctjmp_right:%d  pdctseq_right:%d\n",
            totalpdct,pdctjmp,pdctseq,pdctjmp_right,pdctseq_right);
        printf("predict success rate:%.1f%%\n\n",float(pdctjmp_right+pdctseq_right)/totalpdct*100);
        return 1;
    }
    unsigned nextseqpc=pc+4;
    IF_IDR.curpc=pc;
    IF_IDR.nextseqpc=nextseqpc;
    IF_IDR.ins=ins;
    pc+=4;
    return 0;
}
unsigned ID(){
    unsigned lprbm1=0,lprbm2=0;
    //opcode
    unsigned lui_op=0b0110111,auipc_op=0b0010111,jal_op=0b1101111,jalr_op=0b1100111,
        b_op=0b1100011,l_op=0b0000011,s_op=0b0100011,i_op=0b0010011,r_op=0b0110011;
    if((IF_IDR.ins&0x7F)==lui_op)             ID_EXR.kind=1;//lui
    else if((IF_IDR.ins&0x7F)==auipc_op)      ID_EXR.kind=2;//auipc
    else if((IF_IDR.ins&0x7F)==jal_op)        ID_EXR.kind=3;//jal
    else if((IF_IDR.ins&0x7F)==jalr_op)       ID_EXR.kind=4;//jalr
    else if((IF_IDR.ins&0x7F)==b_op)          ID_EXR.kind=5;//b
    else if((IF_IDR.ins&0x7F)==l_op)          ID_EXR.kind=6;//l
    else if((IF_IDR.ins&0x7F)==s_op)          ID_EXR.kind=7;//s
    else if((IF_IDR.ins&0x7F)==i_op)          ID_EXR.kind=8;//i
    else if((IF_IDR.ins&0x7F)==r_op)          ID_EXR.kind=9;//r

    //rs1
    unsigned rs1=(IF_IDR.ins&0x000F8000)>>15;
    ID_EXR.rs1v=reg[rs1];
    
    //rs2
    unsigned rs2=(IF_IDR.ins&0x01F00000)>>20;
    ID_EXR.rs2v=reg[rs2];

    //data hazard
    if(ID_EXR.kind!=1&&ID_EXR.kind!=2&&ID_EXR.kind!=3){
        if(rs1!=0){
            if(rs1==EX_MEMR.rd&&EX_MEMR.rdw==true){
                ID_EXR.rs1v=EX_MEMR.rdv;
            }
            else if(rs1==EX_MEMR.rd&&EX_MEMR.memvisit==true&&EX_MEMR.meml==true){
                //l inst finished ex;mem unfinished 
                lprbm1=1;
            }
            else if(rs1==MEM_WBR.rd&&MEM_WBR.rdw==true){
                ID_EXR.rs1v=MEM_WBR.rdv;
            }
        }
        if(ID_EXR.kind==5||ID_EXR.kind==7||ID_EXR.kind==9){
            if(rs2!=0){
                if(rs2==EX_MEMR.rd&&EX_MEMR.rdw==true){
                    ID_EXR.rs2v=EX_MEMR.rdv;
                }
                else if(rs2==EX_MEMR.rd&&EX_MEMR.memvisit==true&&EX_MEMR.meml==true){
                    //l inst finished ex;mem unfinished 
                    lprbm2=1;
                }
                else if(rs2==MEM_WBR.rd&&MEM_WBR.rdw==true){
                    ID_EXR.rs2v=MEM_WBR.rdv;
                }
            }
        }
    }
    
    
    //rd
    ID_EXR.rd=(IF_IDR.ins&0x00000F80)>>7;

    //imm
    if(ID_EXR.kind==1||ID_EXR.kind==2){//u-type
        ID_EXR.imm=IF_IDR.ins;//12-31
        ID_EXR.imm=ID_EXR.imm&0xFFFFF000;//0-11
    }
    else if(ID_EXR.kind==3){//j-type
        ID_EXR.imm=((IF_IDR.ins&0x7FE00000)>>20)|((IF_IDR.ins&0x00100000)>>9)
            |(IF_IDR.ins&0x000FF000)|(int(IF_IDR.ins&0x80000000)>>11);
    }
    else if(ID_EXR.kind==4||ID_EXR.kind==6||ID_EXR.kind==8){//i-type
        ID_EXR.imm=int(IF_IDR.ins)>>20;
    }
    else if(ID_EXR.kind==5){//b-type
        ID_EXR.imm=((IF_IDR.ins&0x00000F00)>>7)|((IF_IDR.ins&0x7E000000)>>20)
            |((IF_IDR.ins&0x00000080)<<4)|(int(IF_IDR.ins&0x80000000)>>19);
    }
    else if(ID_EXR.kind==7){//s-type
        ID_EXR.imm=((IF_IDR.ins&0x00000F80)>>7)|(int(IF_IDR.ins&0xFE000000)>>20);
    }
    else if(ID_EXR.kind==9){//r-type
        //no imm
    }

    //func
    ID_EXR.func=((IF_IDR.ins&0x00007000)>>12)|((IF_IDR.ins&0x40000000)>>27);

    //pc
    ID_EXR.curpc=IF_IDR.curpc;
    ID_EXR.nextseqpc=IF_IDR.nextseqpc;
    
    //control hazard
    //jal
    if(ID_EXR.kind==3){
        unsigned jmpaddr=ID_EXR.imm+ID_EXR.curpc;
        pc=jmpaddr;
        return 3;
    }
    //jalr
    if(ID_EXR.kind==4){
        if(lprbm1==0){
            unsigned jmpaddr=(ID_EXR.imm+ID_EXR.rs1v)&0xFFFFFFFE;
            pc=jmpaddr;
            if(lprbm2==0) return 3;
            else return 3+4+4;
        }
        else{
            pc=IF_IDR.curpc;
            ID_EXR.kind=8;
            ID_EXR.imm=0;
            ID_EXR.rs1v=0;
            ID_EXR.rs2v=0;
            ID_EXR.rd=0;
            ID_EXR.func=0;
            if(lprbm2==0) return 3+4;
            else return 3+4+4+4;
        }
    }
    //b
    if(ID_EXR.kind==5){
        bool jmp=predict();
        totalpdct++;
        if(jmp){
            pdctjmp++;
            unsigned jmpaddr=ID_EXR.imm+ID_EXR.curpc;
            pc=jmpaddr;
            if(lprbm1==0&&lprbm2==0) return 1;
            else if(lprbm1==1&&lprbm2==0) return 1+4;
            else if(lprbm1==0&&lprbm2==1) return 1+4+4;
            else return 1+4+4+4;
        }
        else{
            pdctseq++;
            if(lprbm1==0&&lprbm2==0) return 2;
            else if(lprbm1==1&&lprbm2==0) return 2+4;
            else if(lprbm1==0&&lprbm2==1) return 2+4+4;
            else return 2+4+4+4;
        }
    }
    if(lprbm1==0&&lprbm2==0) return 0;
    else if(lprbm1==1&&lprbm2==0) return 0+4;
    else if(lprbm1==0&&lprbm2==1) return 0+4+4;
    else return 0+4+4+4;
}
bool predict(){
    if(dbitspdct==0) return 0;
    else if(dbitspdct==1) return 0;
    else if(dbitspdct==2) return 1;
    else if(dbitspdct==3) return 1;
    return 0;
}
void EX(){
    EX_MEMR.nextseqpc=ID_EXR.nextseqpc;
    switch(ID_EXR.kind){
        case 1:{//lui
            EX_MEMR.rd=ID_EXR.rd;
            EX_MEMR.rdv=ID_EXR.imm;
            EX_MEMR.rdw=true;
            EX_MEMR.jmp=false;
            EX_MEMR.memvisit=false;
            break;
        }
        case 2:{//auipc
            EX_MEMR.rd=ID_EXR.rd;
            EX_MEMR.rdv=ID_EXR.imm+ID_EXR.curpc;
            EX_MEMR.rdw=true;
            EX_MEMR.jmp=false;
            EX_MEMR.memvisit=false;
            break;
        }
        case 3:{//jal
            EX_MEMR.jmpaddr=ID_EXR.imm+ID_EXR.curpc;
            EX_MEMR.jmp=true;
            EX_MEMR.rd=ID_EXR.rd;
            EX_MEMR.rdv=ID_EXR.nextseqpc;
            EX_MEMR.rdw=true;
            EX_MEMR.memvisit=false;
            break;
        }
        case 4:{//jalr
            EX_MEMR.jmpaddr=(ID_EXR.imm+ID_EXR.rs1v)&0xFFFFFFFE;
            EX_MEMR.jmp=true;
            EX_MEMR.rd=ID_EXR.rd;
            EX_MEMR.rdv=ID_EXR.nextseqpc;
            EX_MEMR.rdw=true;
            EX_MEMR.memvisit=false;
            break;
        }
        case 5:{//b
            EX_MEMR.jmpaddr=ID_EXR.imm+ID_EXR.curpc;
            EX_MEMR.rdw=false;
            EX_MEMR.memvisit=false;
            ID_EXR.func=ID_EXR.func&0x7;
            switch(ID_EXR.func){
                case 0:{//beq
                    if(ID_EXR.rs1v==ID_EXR.rs2v){
                        EX_MEMR.jmp=true;
                    }
                    else{
                        EX_MEMR.jmp=false;
                    }
                    break;
                }
                case 1:{//bne
                    if(ID_EXR.rs1v!=ID_EXR.rs2v){
                        EX_MEMR.jmp=true;
                    }
                    else{
                        EX_MEMR.jmp=false;
                    }
                    break;
                }
                case 4:{//blt
                    if(int(ID_EXR.rs1v)<int(ID_EXR.rs2v)){
                        EX_MEMR.jmp=true;
                    }
                    else{
                        EX_MEMR.jmp=false;
                    }
                    break;
                }
                case 5:{//bge
                    if(int(ID_EXR.rs1v)>=int(ID_EXR.rs2v)){
                        EX_MEMR.jmp=true;
                    }
                    else{
                        EX_MEMR.jmp=false;
                    }
                    break;
                }
                case 6:{//bltu
                    if(ID_EXR.rs1v<ID_EXR.rs2v){
                        EX_MEMR.jmp=true;
                    }
                    else{
                        EX_MEMR.jmp=false;
                    }
                    break;
                }
                case 7:{//bgeu
                    if(ID_EXR.rs1v>=ID_EXR.rs2v){
                        EX_MEMR.jmp=true;
                    }
                    else{
                        EX_MEMR.jmp=false;
                    }
                    break;
                }
            }
            break;
        }
        case 6:{//l
            EX_MEMR.memaddr=ID_EXR.rs1v+ID_EXR.imm;
            EX_MEMR.memvisit=true;
            EX_MEMR.meml=true;
            EX_MEMR.mems=false;
            EX_MEMR.memlen=ID_EXR.func&0x3;
            EX_MEMR.memu=ID_EXR.func&0x4;
            EX_MEMR.memrmcycs=2;
            EX_MEMR.rd=ID_EXR.rd;
            EX_MEMR.rdw=false;
            EX_MEMR.jmp=false;
            break;
        }
        case 7:{//s
            EX_MEMR.memaddr=ID_EXR.rs1v+ID_EXR.imm;
            EX_MEMR.memvisit=true;
            EX_MEMR.meml=false;
            EX_MEMR.mems=true;
            EX_MEMR.memdata=ID_EXR.rs2v;
            EX_MEMR.memlen=ID_EXR.func&0x3;
            EX_MEMR.memrmcycs=2;
            EX_MEMR.rdw=false;
            EX_MEMR.jmp=false;
            break;
        }
        case 8:{//i
            EX_MEMR.jmp=false;
            EX_MEMR.memvisit=false;
            EX_MEMR.rdw=true;
            EX_MEMR.rd=ID_EXR.rd;
            unsigned func=ID_EXR.func&0x7;
            switch(func){
                case 0:{//addi
                    EX_MEMR.rdv=ID_EXR.imm+ID_EXR.rs1v;
                    break;
                }
                case 2:{//slti
                    if(int(ID_EXR.rs1v)<int(ID_EXR.imm)){
                        EX_MEMR.rdv=1;
                    }
                    else{
                        EX_MEMR.rdv=0;
                    }
                    break;
                }
                case 3:{//sltiu
                    if(ID_EXR.rs1v<ID_EXR.imm){
                        EX_MEMR.rdv=1;
                    }
                    else{
                        EX_MEMR.rdv=0;
                    }
                    break;
                }
                case 4:{//xori
                    EX_MEMR.rdv=ID_EXR.imm^ID_EXR.rs1v;
                    break;
                }
                case 6:{//ori
                    EX_MEMR.rdv=ID_EXR.imm|ID_EXR.rs1v;
                    break;
                }
                case 7:{//andi
                    EX_MEMR.rdv=ID_EXR.imm&ID_EXR.rs1v;
                    break;
                }
                case 1:{//slli
                    EX_MEMR.rdv=ID_EXR.rs1v<<(ID_EXR.imm&0x1F);
                    break;
                }
                case 5:{
                    switch(ID_EXR.func>>3){
                        case 0:{//srli
                            EX_MEMR.rdv=ID_EXR.rs1v>>(ID_EXR.imm&0x1F);
                            break;
                        }
                        case 1:{//srai
                            EX_MEMR.rdv=int(ID_EXR.rs1v)>>(ID_EXR.imm&0x1F);
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
        case 9:{//r
            EX_MEMR.jmp=false;
            EX_MEMR.memvisit=false;
            EX_MEMR.rdw=true;
            EX_MEMR.rd=ID_EXR.rd;
            unsigned func=ID_EXR.func&0x7;
            switch(func){
                case 0:{
                    switch(ID_EXR.func>>3){
                        case 0:{//add
                            EX_MEMR.rdv=ID_EXR.rs1v+ID_EXR.rs2v;
                            break;
                        }
                        case 1:{//sub
                            EX_MEMR.rdv=ID_EXR.rs1v-ID_EXR.rs2v;
                            break;
                        }
                    }
                    break;
                }
                case 1:{//sll
                    EX_MEMR.rdv=ID_EXR.rs1v<<(ID_EXR.rs2v&0x1F);
                    break;
                }
                case 2:{//slt
                    if(int(ID_EXR.rs1v)<int(ID_EXR.rs2v)){
                        EX_MEMR.rdv=1;
                    }
                    else{
                        EX_MEMR.rdv=0;
                    }
                    break;
                }
                case 3:{//sltu
                    if(ID_EXR.rs1v<ID_EXR.rs2v){
                        EX_MEMR.rdv=1;
                    }
                    else{
                        EX_MEMR.rdv=0;
                    }
                    break;
                }
                case 4:{//xor
                    EX_MEMR.rdv=ID_EXR.rs1v^ID_EXR.rs2v;
                    break;
                }
                case 5:{
                    switch(ID_EXR.func>>3){
                        case 0:{//srl
                            EX_MEMR.rdv=ID_EXR.rs1v>>(ID_EXR.rs2v&0x1F);
                            break;
                        }
                        case 1:{//sra
                            EX_MEMR.rdv=int(ID_EXR.rs1v)>>(ID_EXR.rs2v&0x1F);
                            break;
                        }
                    }
                    break;
                }
                case 6:{
                    EX_MEMR.rdv=ID_EXR.rs1v|ID_EXR.rs2v;
                    break;
                }
                case 7:{
                    EX_MEMR.rdv=ID_EXR.rs1v&ID_EXR.rs2v;
                    break;
                }
            }
            break;
        }
    }
    
}
void MEM(){ 
    if(!EX_MEMR.memvisit){ 
        //reg
        if(EX_MEMR.rdw){
            MEM_WBR.rdw=true;
            MEM_WBR.rd=EX_MEMR.rd;
            MEM_WBR.rdv=EX_MEMR.rdv;
        }
    }
    else{
        switch(EX_MEMR.memrmcycs){
            case 2:{
                EX_MEMR.memrmcycs-=1;
                //congest
                congestmat[congestptr][2]=1;
                congestmat[congestptr][3]=1;
                congestmat[congestptr][4]=1;
                //nop
                MEM_WBR.rd=0;
                MEM_WBR.rdv=0;
                MEM_WBR.rdw=1;
                break;
            }
            case 1:{
                EX_MEMR.memrmcycs-=1;
                //congest
                congestmat[congestptr][2]=1;
                congestmat[congestptr][3]=1;
                congestmat[congestptr][4]=1;
                //nop
                MEM_WBR.rd=0;
                MEM_WBR.rdv=0;
                MEM_WBR.rdw=1;
                break;
            }
            case 0:{
                
                if(EX_MEMR.meml){
                    MEM_WBR.rd=EX_MEMR.rd;
                    MEM_WBR.rdw=true;
                    switch(EX_MEMR.memlen){
                        case 0:{
                            if(EX_MEMR.memu){//lbu
                                MEM_WBR.rdv=memory[EX_MEMR.memaddr]&0xFF;
                            }
                            else{//lb
                                MEM_WBR.rdv=int((memory[EX_MEMR.memaddr]&0xFF)<<24)>>24;
                            }
                            break;
                        }
                        case 1:{
                            if(EX_MEMR.memu){//lhu
                                MEM_WBR.rdv=(memory[EX_MEMR.memaddr]&0xFF)|((memory[EX_MEMR.memaddr+1]&0xFF)<<8);
                            }
                            else{//lh
                                MEM_WBR.rdv=(int((memory[EX_MEMR.memaddr]&0xFF)|((memory[EX_MEMR.memaddr+1]&0xFF)<<8))<<16)>>16;
                            }
                            break;
                        }
                        case 2:{//lw
                            MEM_WBR.rdv=(memory[EX_MEMR.memaddr]&0xFF)|((memory[EX_MEMR.memaddr+1]&0xFF)<<8)
                                        |((memory[EX_MEMR.memaddr+2]&0xFF)<<16)|((memory[EX_MEMR.memaddr+3]&0xFF)<<24);
                            break;
                        }
                    }
                }
                else if(EX_MEMR.mems){
                    MEM_WBR.rdw=false;
                    switch(EX_MEMR.memlen){
                        case 0:{//sb
                            memory[EX_MEMR.memaddr]=EX_MEMR.memdata&0xFF;
                            break;
                        }
                        case 1:{//sh
                            memory[EX_MEMR.memaddr]=EX_MEMR.memdata&0xFF;
                            memory[EX_MEMR.memaddr+1]=(EX_MEMR.memdata&0xFF00)>>8;
                            break;
                        }
                        case 2:{//sw
                            memory[EX_MEMR.memaddr]=EX_MEMR.memdata&0xFF;
                            memory[EX_MEMR.memaddr+1]=(EX_MEMR.memdata&0xFF00)>>8;
                            memory[EX_MEMR.memaddr+2]=(EX_MEMR.memdata&0xFF0000)>>16;
                            memory[EX_MEMR.memaddr+3]=(EX_MEMR.memdata&0xFF000000)>>24;
                            break;
                        }
                    }
                }
                break;
            }
        }
    } 
}
void WB(){
    if(MEM_WBR.rdw){
        if(MEM_WBR.rd!=0){
            reg[MEM_WBR.rd]=MEM_WBR.rdv;
        }
    }
    ;
}
void read(const char*filepath){
    FILE*fp;
    if((fp=fopen(filepath,"r"))==NULL){
        printf("invalid filepath.");
        exit(0);
    }
    int baseaddr,addr;
    while(true){
        char buf[100];
        buf[0]=fgetc(fp);
        if(feof(fp)) break;
        int k=1;
        while((buf[k++]=fgetc(fp))!='\n');
        buf[k]='\0';
        if(buf[0]=='@'){
            sscanf(buf,"@%x\n",&baseaddr);
            addr=baseaddr;
            continue;
        }
        else{
            k=0;
            int t1,t2,t3,t4;
            while(buf[k]!='\n'){
                sscanf(buf+k,"%x %x %x %x",&t1,&t2,&t3,&t4);
                memory[addr]=t1;
                memory[addr+1]=t2;
                memory[addr+2]=t3;
                memory[addr+3]=t4;
                addr+=4;
                k+=12;
            }
        }
    }
    fclose(fp);
}