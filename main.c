

#include <stdio.h>
#include <string.h>

char *readTitle(char *Filename);
char *readNextA(char *Filename);
char *readNextB(char *Filename);
char *readText(char *Filename);
char temp[100];
char Starttxt[100] = "Start";

int main()
{

  ;
  struct
  {
      char Title[100];
      char nTitleA[100];
      char nTitleB[100];
      char Text[1000];
  } textBlock[10];


  /*strcpy(textBlock[0].Title,readTitle(Starttxt));
  strcpy(textBlock[0].nTitleA,readNextA(Starttxt));
  strcpy(textBlock[0].nTitleB,readNextB(Starttxt));
  strcpy(textBlock[0].Text,readText(Starttxt));
*/




  for (int x = 0; x<3; x++)
  {
    strcat(Starttxt,".txt");
    strcpy(textBlock[x].Title,readTitle(Starttxt));
    strcpy(textBlock[x].nTitleA,readNextA(Starttxt));
    strcpy(textBlock[x].nTitleB,readNextB(Starttxt));
    strcpy(textBlock[x].Text,readText(Starttxt));
    strcpy(Starttxt,textBlock[x].Title);
    strcat(Starttxt,".txt"); // \n muss no weg
    printf("%d. Startxt is: %s", x+1, Starttxt);
    printf("%d. Title: %s", x+1, textBlock[x].Title);
    printf("%d. TitleA: %s", x+1, textBlock[x].nTitleA);
  }


return 0;
}



char *readTitle(char *Filename)
{

  FILE *input = fopen(Filename, "r");  // read
  if (input)   // file opened OK
  {

      fgets(temp, 100, input);

    return temp;

  }else
  {
    printf("Input missing");
  }
  fclose(input);
}
char *readNextA(char *Filename)
{

  FILE *input = fopen(Filename, "r");  // read
  if (input)   // file opened OK
  {
    char dump[100];
    fgets(dump, 100, input);
    fgets(temp, 100, input);

    return temp;

  }else
  {
    printf("Input missing");
  }
  fclose(input);
}

char *readNextB(char *Filename)
{

  FILE *input = fopen(Filename, "r");  // read
  if (input)   // file opened OK
  {
    char dump[100];
    fgets(dump, 100, input);
    fgets(dump, 100, input);
    fgets(temp, 100, input);

    return temp;

  }else
  {
    printf("Input missing");
  }
  fclose(input);
}

char *readText(char *Filename)
{

  FILE *input = fopen(Filename, "r");  // read
  if (input)   // file opened OK
  {
    char dump[100];
    fgets(dump, 100, input);
    fgets(dump, 100, input);
    fgets(dump, 100, input);

    while(!feof(input)){
      char chain[100];
      fgets(chain, 100, input);
      strcat(temp,chain);
      }

    return temp;

  }else
  {
    printf("Input missing");
  }
  fclose(input);
}