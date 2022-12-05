
#include <stdio.h> 
#include <string.h>

#define MAXLINELEN 	20
#define NUM_ASSC	8
#define NUM_SETS	0x8000
#define MAX       3
#define ID_SHIFT	6	
#define ID_MASK	0x3FFFC0
#define TAG_SHIFT	21
#define TAG_MASK	0xFFE00000

#define ADDRID(x)		((x << ID_SHIFT) & ID_MASK)
#define ADDRTAG(x)		((x << TAG_SHIFT) & TAG_MASK)

int validChar(char NewAddr)
{
   
   int one = 1, two = 0; 

   printf("New Address?: ") ;
   scanf(" %c", &NewAddr);

   if(NewAddr == 'y')
   {
      return one; 
   }
   else if( NewAddr == 'n'  )
   { 
      return two;
   }
   else 
   { 
        printf("invalid address, Enter new address?");
        scanf(" %c", &NewAddr);

   }
}


int main()
{
   int num, cmd, tag, id, addr, byte; 
   char NewAddr;
   FILE *fptr;

   // use appropriate location if you are using MacOS or Linux
   fptr = fopen("trace.txt", "w");

   if(fptr == NULL)
   {
      printf("Error! \n");   
      return 1;              
   }


while(validChar(NewAddr) == 1)
{
   
   //enter cmd
   printf("Enter Command:  ");
   scanf(" %d",&cmd);
   if(cmd < 0  || cmd > 9 )
   {
      printf("Invalid Command: Enter new command:  ");
      scanf(" %d",&cmd);
   }
   //Enter a tag
   printf("Enter tag:  "); 
   scanf(" %d",&tag);
      if(tag < 0  || tag > 2048 )
   {
      printf("Invalid tag! Enter new tag:  ");
      scanf(" %d",&tag);
   }
           
   //enter an index value
   printf("Enter ID:  "); 
   scanf(" %d",&id);  
   if(id < 0 || id > 32768)
   {
      printf("INVALID ID! Enter new ID:  "); 
      scanf(" %d",&id); 
   }

   printf("Enter byte select:  "); 
   scanf(" %d",&byte);  
   if(byte < 0 || byte > 64)
   {
      printf("INVALID byte select! Enter new byte select:  "); 
      scanf(" %d",&id); 
   }

   //create new address 
   id = ADDRID(id); 
   tag = ADDRTAG(tag);
   addr = id + tag + byte; 
   //send address to file
   fprintf(fptr,"%d %x \n", cmd, addr);

}

   fclose(fptr);

   return 0;

}


