

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#define LINE_BUFFER_SIZE 64

typedef struct _Chapter_
{
    char *title_;
    char *text_;
    struct _Chapter_ *choiceA_;
    struct _Chapter_ *choiceB_;
} Chapter;

int printChapter(Chapter *chapter);
char getInput();
char *readLine(FILE *fin);
Chapter *loadChapter(char *filename);
char *readAll(FILE *fin);



void adventure()
{
  loadChapter("Start");

  while(printChapter())
  {
    getInput();
  }
}


int printChapter(Chapter *chapter)
{
  if(chapter->choiceA_== NULL)
  {
    printf("------------------------------\n"
           "%s\n"
           "\n"
           "%s\n"
           "\n"
           "ENDE\n)", chapter->title_, chapter->text_);

    return 0;
  }
  else
  {
    printf("------------------------------\n"
           "%s\n"
           "\n"
           "%s\n"
           "\n"
           "Deine Wahl (A/B)? ", chapter->title_,chapter->text_);

    return 1;
  }

}


char getInput()
{
  while(1)
  {
    char result = (char) getchar();
    if (result == 'A' || result == 'B')
    {
      return result;
    }
  }
}


Chapter *loadChapter(char *filename)
{
  FILE *fin = fopen(filename, "r");

  Chapter *chapter = malloc(sizeof(Chapter));
  chapter->title_ = readLine(fin);

  char *testA = readLine(fin);
  if(testA[0] != '-' && testA[1] != '\0')
  {
    chapter->choiceA_ = loadChapter(testA);
  }
  else
  {
    chapter->choiceA_ = NULL;
  }
  free(testA);

  char *testB = readLine(fin);
  if(testB[0] != '-' && testB[1] != '\0')
  {
    chapter->choiceB_ = loadChapter(testB);
  }
  else
  {
    chapter->choiceA_ = NULL;
  }
  free(testB);

  chapter->text_ = readAll(fin);

  fclose(fin);

  return chapter;
}

char *readLine(FILE *fin)
{
  char c;
  char buffer[LINE_BUFFER_SIZE];
  int i = 0;
  int length = 0;

  char *line = malloc(LINE_BUFFER_SIZE);

  while((c = (char)fgetc(fin)) != '\n')
  {
    if(c == '\r')
    {
      continue;
    }

    buffer[i] = c;
    i++;
    length++;

    if (i == LINE_BUFFER_SIZE)
    {
      memmove(line + (length - i), buffer, (size_t) i);
      line = realloc(line, (size_t) length);
      i = 0;
    }
  }

  memmove(line + (length - i), buffer, (size_t) i);
  line = realloc(line, (size_t) (length + 1));
  line[length] = '\0';

  return line;
}

char *readAll(FILE *fin)
{
  int c;
  char buffer[LINE_BUFFER_SIZE];
  int i = 0;
  int length = 0;

  char *line = malloc(LINE_BUFFER_SIZE);

  while((c = fgetc(fin)) != EOF)
  {
    if((char) c == '\r')
    {
      continue;
    }

    buffer[i] = (char) c;
    i++;
    length++;

    if (i == LINE_BUFFER_SIZE)
    {
      memmove(line + (length - i), buffer, (size_t) i);
      line = realloc(line, (size_t) length);
      i = 0;
    }
  }

  memmove(line + (length - i), buffer, (size_t) i);
  line = realloc(line, (size_t) (length + 1));
  line[length] = '\0';

  return line;
}
