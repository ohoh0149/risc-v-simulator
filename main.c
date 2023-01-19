#include <stdio.h>
#include <string.h>

int get_opcode(int instruction)
{
  return instruction & 0x7F;
}
int get_func3(int instruction)
{
  return (instruction >> 12) & 0x7;
}
int get_func7(int instruction)
{
  return (instruction >> 25) & 0x7F;
}
int get_rd(int instruction)
{
  return (instruction >> 7) & 0x1F;
}
int get_rs1(int instruction)
{
  return (instruction >> 15) & 0x1F;
}
int get_rs2(int instruction)
{
  return (instruction >> 20) & 0x1F;
}
int get_immediate(int instruction)
{
  const int opcode = get_opcode(instruction);
  const int func7 = get_func7(instruction);
  const int func3 = get_func3(instruction);

  if (opcode == 0x13 || opcode == 0x3 || opcode == 0x67)
  { // I
    if (func3==3&&func7 == 0x20)
    { // srai
      return (instruction >> 20) & 0x1f;
    }
    else
    {
      return instruction >> 20;
    }
  }
  else if (opcode == 0x23)
  { // S
    return ((instruction >> 7) & 0x1F) | ((instruction >> 25) << 5);
  }
  else if (opcode == 0x37 || opcode == 0x17)
  { // U
    return (instruction >> 12) << 12;
  }
  else if (opcode == 0x6F)
  { // UJ
    return (((instruction >> 21) & 0x3ff) | (((instruction >> 20) & 0x1) << 10) | ((instruction >> 12) & 0xff) << 11 | ((instruction >> 31) << 19)) << 1;
  }
  else if (opcode == 0x63)
  { // SB
    return (((instruction >> 8) & 0xf) | ((instruction >> 25) & 0x3f) << 4 | ((instruction >> 7) & 0x1) << 10 | ((instruction >> 31) << 11)) << 1;
  }
  else
  {
    return -999;
  }
}

char *get_instruction(int instruction)
{
  int opcode = get_opcode(instruction);
  int func3 = get_func3(instruction);
  int func7 = get_func7(instruction);

  if (opcode == 0x33)
  {
    if (func3 == 0x0)
    {
      if (func7 == 0x0)
      {
        return "add";
      }
      else if (func7 == 0x20)
      {
        return "sub";
      }
    }
    else if (func3 == 0x1)
    {
      return "sll";
    }
    else if (func3 == 0x2)
    {
      return "slt";
    }
    else if (func3 == 0x3)
    {
      return "sltu";
    }
    else if (func3 == 0x4)
    {
      return "xor";
    }
    else if (func3 == 0x5)
    {
      if (func7 == 0x0)
      {
        return "srl";
      }
      else if (func7 == 0x20)
      {
        return "sra";
      }
    }
    else if (func3 == 0x6)
    {
      return "or";
    }
    else if (func3 == 0x7)
    {
      return "and";
    }
  }
  else if (opcode == 0x13)
  {
    if (func3 == 0x0)
    {
      return "addi";
    }
    else if (func3 == 0x1)
    {
      return "slli";
    }
    else if (func3 == 0x2)
    {
      return "slti";
    }
    else if (func3 == 0x3)
    {
      return "sltiu";
    }
    else if (func3 == 0x4)
    {
      return "xori";
    }
    else if (func3 == 0x5)
    {
      if (func7 == 0x0)
      {
        return "srli";
      }
      else if (func7 == 0x20)
      {
        return "srai";
      }
    }
    else if (func3 == 0x6)
    {
      return "ori";
    }
    else if (func3 == 0x7)
    {
      return "andi";
    }
  }
  else if (opcode == 0x3)
  {
    if (func3 == 0x0)
    {
      return "lb";
    }
    else if (func3 == 0x1)
    {
      return "lh";
    }
    else if (func3 == 0x2)
    {
      return "lw";
    }
    else if (func3 == 0x4)
    {
      return "lbu";
    }
    else if (func3 == 0x5)
    {
      return "lhu";
    }
  }
  else if (opcode == 0x23)
  { // S
    if (func3 == 0x0)
    {
      return "sb";
    }
    else if (func3 == 0x1)
    {
      return "sh";
    }
    else if (func3 == 0x2)
    {
      return "sw";
    }
  }
  else if (opcode == 0x37)
  { // U
    return "lui";
  }
  else if (opcode == 0x17)
  { // U
    return "auipc";
  }
  else if (opcode == 0x6F)
  { // UJ
    return "jal";
  }
  else if (opcode == 0x67)
  { // I
    return "jalr";
  }
  else if (opcode == 0x63)
  { // SB
    if (func3 == 0x0)
    {
      return "beq";
    }
    else if (func3 == 0x1)
    {
      return "bne";
    }
    else if (func3 == 0x4)
    {
      return "blt";
    }
    else if (func3 == 0x5)
    {
      return "bge";
    }
    else if (func3 == 0x6)
    {
      return "bltu";
    }
    else if (func3 == 0x7)
    {
      return "bgeu";
    }
  }

  return "unknown instruction";
}

int reg[32] = {
    0,
};
int pc = 0;
void set_reg(int i, int v)
{
  reg[i] = v;
  reg[0] = 0;
}
int instruction_mem[20000];
int data_mem[20000];
char* data_mem_byte=data_mem;
//char temp_mem;
int main(int argc, char **argv)
{
  for (int i = 0; i < 20000; i++)
  {
    instruction_mem[i] = 0xffffffff;
    data_mem[i]=0xffffffff;
  }
  int count = 0;
  FILE *instruction_file=NULL;
  FILE *data_file=NULL;

  if (argc == 3)
  {
    instruction_file = fopen(argv[1], "r");
    if (sscanf(argv[2], "%i", &count) != 1)
    {
      printf("Please enter an integer to second argument");
    }
  }
  else if (argc == 4)
  {
    instruction_file = fopen(argv[1], "r");
    data_file = fopen(argv[2], "r");
    if (sscanf(argv[3], "%i", &count) != 1)
    {
      printf("Please enter an integer to third argument");
    }
    for (int i = 0;; i++)
    {
      int flag = fread(&data_mem[i], sizeof(int), 1, data_file);
      if (flag == 0)
        break;
    }
  }
  for (int i = 0;; i++)
  {
    int flag = fread(&instruction_mem[i], sizeof(int), 1, instruction_file);
    if (flag == 0)
      break;
  }

  int data;
  int opcode = 0;
  

  if (NULL != instruction_file)
  {
     for(int i=0;i<count;i++)

    {
      // if (fread(&instruction, sizeof(int), 1, file) == 0)
      // {
      //   printf("No more instructions\n");
      //   break;
      // }
      // else
      // {
        data=instruction_mem[pc/4];
        if(pc>=0x00010000)
        {
          printf("Memory address error\n");
          pc+=4;
          break;
        }
        if(data==0xffffffff)
        {
          printf("unknown instruction\n");
          pc+=4;
          break;
          
          
        }
        else
      {
        int op = get_opcode(data);
        int rd = get_rd(data);
        int rs1 = get_rs1(data);
        int rs2 = get_rs2(data);
        int immediate = get_immediate(data);
        char *instruction = get_instruction(data);

        if (op == 0x33) // R type
        {

          if (strcmp(instruction, "and") == 0)
          {
            set_reg(rd, reg[rs1] & reg[rs2]);
          }
          else if (strcmp(instruction, "or") == 0)
          {
            set_reg(rd, reg[rs1] | reg[rs2]);
          }
          else if (strcmp(instruction, "sra") == 0)
          {
            set_reg(rd, reg[rs1] >> (reg[rs2] & 0x1f));
          }
          else if (strcmp(instruction, "srl") == 0)
          {
            set_reg(rd, (unsigned int)reg[rs1] >> (reg[rs2] & 0x1f));
          }
          else if (strcmp(instruction, "xor") == 0)
          {
            set_reg(rd, reg[rs1] ^ reg[rs2]);
          }
          // can overflow check
          else if (strcmp(instruction, "sltu") == 0)
          {
            if ((unsigned int)reg[rs1] < (unsigned int)reg[rs2])
            {
              set_reg(rd, 1);
            }
            else
            {
              set_reg(rd, 0);
            }
          }
          else if (strcmp(instruction, "slt") == 0)
          {
            if (reg[rs1] < reg[rs2])
            {
              set_reg(rd, 1);
            }
            else
            {
              set_reg(rd, 0);
            }
          }
          else if (strcmp(instruction, "sll") == 0)
          {
            set_reg(rd, reg[rs1] << (reg[rs2] & 0x1f));
          }
          else if (strcmp(instruction, "sub") == 0)
          {
            set_reg(rd, reg[rs1] - reg[rs2]);
          }
          else if (strcmp(instruction, "add") == 0)
          {
            set_reg(rd, reg[rs1] + reg[rs2]);
          }
        }

        else if (op == 0x13) // I type addi,slli,slti,sltiu,xori,srli,srai,ori,andi,
        {
          if (strcmp(instruction, "addi") == 0)
          {

            set_reg(rd, reg[rs1] + immediate);
          }
          else if (strcmp(instruction, "slti") == 0)
          {
            if (reg[rs1] < immediate)
            {
              set_reg(rd, 1);
            }
            else
            {
              set_reg(rd, 0);
            }
          }
          else if (strcmp(instruction, "sltiu") == 0)
          {
            if ((unsigned int)reg[rs1] < (unsigned int)immediate)
            {
              set_reg(rd, 1);
            }
            else
            {
              set_reg(rd, 0);
            }
          }
          else if (strcmp(instruction, "xori") == 0)
          {
            set_reg(rd, reg[rs1] ^ immediate);
          }
          else if (strcmp(instruction, "ori") == 0)
          {
            set_reg(rd, reg[rs1] | immediate);
          }
          else if (strcmp(instruction, "andi") == 0)
          {
            set_reg(rd, reg[rs1] & immediate);
          }
          else if (strcmp(instruction, "slli") == 0)
          {
            set_reg(rd, reg[rs1] << (immediate & 0x1f));
          }
          else if (strcmp(instruction, "srli") == 0)
          {
            set_reg(rd, (unsigned int)reg[rs1] >> (immediate & 0x1f));
          }
          else if (strcmp(instruction, "srai") == 0)
          {
            set_reg(rd, reg[rs1] >> (immediate & 0x1f));
          }
        }
        else if (op == 0x67 || op == 0x3) // I type
        {
         if (strcmp(instruction, "lw") == 0)
          {
            
            set_reg(rd,data_mem[(reg[rs1]+immediate-0x10000000)/4]);
            
          }
          if (strcmp(instruction, "lh") == 0)
          {
            
            set_reg(rd,data_mem[(reg[rs1]+immediate-0x10000000)/4]&0xffff);
          }
          if (strcmp(instruction, "lhu") == 0)
          {
            
            set_reg(rd,(unsigned short)(data_mem[(reg[rs1]+immediate-0x10000000)/4]&0xffff));
          }
          if (strcmp(instruction, "lb") == 0)
          {

             int address=reg[rs1]+immediate;
             int index=address-0x10000000;
             
             unsigned char temp_data=0;
            temp_data= *(data_mem_byte+index);
            int temp= temp_data;
            set_reg(rd,(temp<<24)>>24);
            
          }
          if (strcmp(instruction, "lbu") == 0)
          {
            int address=reg[rs1]+immediate;
            int index=address-0x10000000;
            char temp_data=0;
            
            
            if((reg[rs1]+immediate)==0x20000000)
            {
              //printf("!!!!");
              scanf("%u",&reg[rd]);
             // scanf("%u",&reg[rd]);
            }
            else{
            
            temp_data= *(data_mem_byte+index);
              
        
            set_reg(rd,temp_data);
            }
          }
          if (strcmp(instruction, "jalr") == 0)
          {
            int temp=pc;
            pc=reg[rs1]+immediate;
            set_reg(rd,temp+4);
            
            pc-=4;
          } 
        }
        else if (op == 0x23) // S type
        {
          if (strcmp(instruction, "sw") == 0)
          {
            data_mem[(reg[rs1]+immediate-0x10000000)/4]=reg[rs2];
          }
          if (strcmp(instruction, "sh") == 0)
          {
            //??????????????????????????????????
            data_mem[(reg[rs1]+immediate-0x10000000)/4]&=0xffff0000;
            data_mem[(reg[rs1]+immediate-0x10000000)/4]|=(reg[rs2]&0xffff);

          }
          if (strcmp(instruction, "sb") == 0)
          {
            int address=reg[rs1]+immediate;
             int index=address-0x10000000;
            //??????????????????????????????????
            if((reg[rs1]+immediate)==0x20000000)
            {
              // printf("%c",(unsigned char)reg[rs2]&0x000000ff);
              printf("%c",(char)reg[rs2]);
            }
            else 
            {
              
              //data_mem[16371]&= (0x00ffffff|((reg[rs2]&0xff)<<24));
              *(data_mem_byte+index)=(unsigned char)reg[rs2];
            }
            // else{
            //    // printf("hihihiihihihi");
            //  //data_mem[(address-0x10000000)/4]&=0xffffff00;
            //  //data_mem[(address-0x10000000)/4]|=(reg[rs2]&0xff);
            // // printf("%c",(char)(data_mem[(reg[rs1]+immediate-0x10000000)/4]));
            // }


          }
          // printf("%s x%d, %d(x%d)\n", get_instruction(instruction), get_rs2(instruction), get_immediate(instruction), get_rs1(instruction));
        }
        else if (op == 0x37 || op == 0x17) // U type
        {
          if (strcmp(instruction, "lui") == 0)
          {
            set_reg(rd, immediate);
          }
          if (strcmp(instruction, "auipc") == 0)
          {
            set_reg(rd, pc+(immediate));
            
          }
          // printf("%s x%d, %d\n", get_instruction(instruction), get_rd(instruction), get_immediate(instruction));
        }
        else if (op == 0x6F) // UJ type
        {
          if (strcmp(instruction, "jal") == 0)
          {
            set_reg(rd,pc+4);
            pc+=immediate;
            pc-=4;
          }
          // printf("%s x%d, %d\n", get_instruction(instruction), get_rd(instruction), get_immediate(instruction));
        }
        else if (op == 0x63) // SB type
        {
          if (strcmp(instruction, "beq") == 0)
          {
            if(reg[rs1]==reg[rs2])
            {
              pc=pc+immediate;
              pc-=4;
            }
          }
          if (strcmp(instruction, "bne") == 0)
          {
            if(reg[rs1]!=reg[rs2])
            {
              pc=pc+immediate;
              pc-=4;
            }
          }
          if (strcmp(instruction, "blt") == 0)
          {
            if(reg[rs1]<reg[rs2])
            {
              pc=pc+immediate;
              pc-=4;
            }
          }
          if (strcmp(instruction, "bge") == 0)
          {
            if(reg[rs1]>=reg[rs2])
            {
              pc=pc+immediate;
              pc-=4;
            }
          }
          if (strcmp(instruction, "bltu") == 0)
          {
            if((unsigned)reg[rs1]<(unsigned)reg[rs2])
            {
              pc=pc+immediate;
              pc-=4;
            }
          }
          if (strcmp(instruction, "bgeu") == 0)
          {
            if((unsigned)reg[rs1]>=(unsigned)reg[rs2])
            {
              pc=pc+immediate;
              pc-=4;
            }
          }
          // printf("%s x%d, x%d, %d\n", get_instruction(instruction), get_rs1(instruction), get_rs2(instruction), get_immediate(instruction));
        }
        else
        {
          // printf("%s\n", get_instruction(instruction));
        }
      }

      pc+=4;
    }

    for (int i = 0; i < 32; i++)
    {
      printf("x%d: 0x%08x\n", i, reg[i]);
    }
    printf("PC: 0x%08x\n",pc);
  }
  if(instruction_file!=NULL)
   fclose(instruction_file);
  if(data_file!=NULL)
    fclose(data_file);
}