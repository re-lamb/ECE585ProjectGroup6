
#include <stdio.h> 
#include <string.h>

#define MAXLINELEN 	20
#define NUM_ASSC	8
#define NUM_SETS	0x8000

#define ID_SHIFT	6	
#define ID_MASK	0x3FFFC0
#define TAG_SHIFT	21
#define TAG_MASK	0xFFE00000

#define ADDRID(x)		((x << ID_SHIFT) & ID_MASK)
#define ADDRTAG(x)		((x << TAG_SHIFT) & TAG_MASK)



int main()
{
   int num, cmd, tag, id, addr; 
   char NewAddr = 'y';
   FILE *fptr;

   // use appropriate location if you are using MacOS or Linux
   fptr = fopen("trace.txt", "w");

   if(fptr == NULL)
   {
      printf("Error! \n");   
      return 1;              
   }

while(NewAddr == 'y' || NewAddr == 'Y'|| NewAddr != 'N' || NewAddr != 'n')
{
   //ask for new address
   printf("Enter new address(y/n):");
   scanf("%c",&NewAddr);
   if(NewAddr == 'n' || NewAddr == 'N')
   {
      break; 
   }

   if((NewAddr != 'y' || NewAddr != 'Y' || NewAddr != 'N' || NewAddr != 'n'))
   {
      printf("Invalid Address: Enter new address(y/n):");
      scanf("%c",&NewAddr);
   }

   //enter cmd
   printf("Enter Command:  ");
   scanf("%d",&cmd);
   if(cmd < 0  || cmd > 9 )
   {
      printf("Invalid Command: Enter new command:  ");
      scanf("%d",&cmd);
   }
   //Enter a tag
   printf("Enter tag:  "); 
   scanf("%d",&tag);
      if(tag < 0  || tag > 2048 )
   {
      printf("Invalid Command: Enter new command:  ");
      scanf("%d",&tag);
   }
           
   //enter an index value
   printf("Enter ID:  "); 
   scanf("%d",&id);  
   if(id < 0 || id > 32768)
   {
      printf("INVALID ID: Enter new ID:  "); 
      scanf("%d",&id); 
   }

   //create new address 
   id = ADDRID(id); 
   tag = ADDRTAG(tag);
   addr = id + tag; 
   //send address to file
   fprintf(fptr,"%d %x \n", cmd, addr);
}

   fclose(fptr);

   return 0;

}


