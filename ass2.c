#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#define FAIL 0
#define OK 1
#define OUT_OF_MEMORY FAIL

#define FAILED(r) ((r) != OK)
#define PRINT_OUT_OF_MEMORY() printf("Out of Memory")

#define LINE_BUFFER_SIZE 128

#define COMPAREFUNCT (compare)



typedef struct _Chapter_
{
    char *title_;
    char *text_;
    void  *choiceA_;
    void  *choiceB_;
} Chapter;

typedef struct _ChapterLink
{
    char *filename;
    Chapter *chapter;
} ChapterLink;


typedef struct _Vector
{
    int capacity;
    int size;
    void **elements;
} Vector;


//basic
int printChapter(Chapter *chapter);
char getInput();
char *readLine(FILE *fin);
int loadChapter(char *filename, Chapter **chapterOut);
char *readAll(FILE *fin);
void adventure(Chapter *next);
void freeChapters(Chapter *chapter);

void loadAllChapters(char *filename,Vector *chapterList, Vector *linkVec, int (*compare)(void*, void*));
int compareStuff(void *valueA, void *valueB);
void linkChapters(Vector *chapterList, Vector *linkVec, int (*compare)(void*,void*));

//vectors
int vecAdd(Vector *vector, void *newEntry);
int vecAddIndex(Vector *vector, void *newEntry, int index);
Vector *vecCreate(int initCapacity);
void vecDelete(Vector *vector, void (*delete)(void*));//TODO
int vecAddOrdered(Vector *vector, void *newEntry, int (*COMPAREFUNCT)(void*, void*)); //TODO
void *vecGet(Vector *vector, void *compareValue, int (*compare)(void*, void*));
int vecTrim(Vector *vector);
int vecEnsureCap(Vector *vector);


int main()
{
  int *loop = 0; //Kreisverweis existent? JA(1)/NEIN(0) //nope

  Vector *chapterList = vecCreate(1);
  Vector *linkVec = vecCreate(1);
  int (*compare) (void*, void*) = &compareStuff;

  loadAllChapters("Start.txt", chapterList, linkVec, compare);
  linkChapters(chapterList, linkVec, compare);


  adventure(chapterList->elements[0]);

  freeChapters(chapterList->elements[0]);

  return 0;
}

void freeChapters(Chapter *chapter)
{
  if (chapter->choiceA_ != NULL)
  {
    freeChapters(chapter->choiceA_);
  }

  if (chapter->choiceB_ != NULL)
  {
    freeChapters(chapter->choiceB_);
  }

  free(chapter->title_);
  free(chapter->text_);
  free(chapter);
}


void adventure(Chapter *next)
{
  while (printChapter(next))
  {
    if (getInput() == 'A')
    {
      next = next->choiceA_;
    }
    else
    {
      next = next->choiceB_;
    }
  }
}

void deleteChapter(Chapter *chapter)
{
  free(chapter->title_);
  free(chapter->text_);
  if (chapter->choiceA_ != NULL)
  {
    deleteChapter(chapter->choiceA_);
  }

  if (chapter->choiceA_ != NULL)
  {
    deleteChapter(chapter->choiceB_);

  }
}

void deleteChapterLink(ChapterLink *chapterLink)
{
  //free(); //TODO
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


int loadChapter(char *filename, Chapter **chapterOut)
{
  FILE *fin = fopen(filename, "r");

  *chapterOut = malloc(sizeof(Chapter));
  (*chapterOut)->title_ = readLine(fin);

  char *testA = readLine(fin);
  if(testA[0] != '-' && testA[1] != '\0')
  {
    (*chapterOut)->choiceA_ = testA;
  }
  else
  {
    (*chapterOut)->choiceA_ = NULL;
  }
  free(testA);

  char *testB = readLine(fin);
  if(testB[0] != '-' && testB[1] != '\0')
  {
    (*chapterOut)->choiceB_ = testB;
  }
  else
  {
    (*chapterOut)->choiceA_ = NULL;
  }
  free(testB);

  (*chapterOut)->text_ = readAll(fin);

  fclose(fin);


  return OK;
}

int compareStuff(void *valueA, void *valueB)
{

  //valueA pointer to filename
  //valueB pointer to chapterlink
  ChapterLink *chapterLink = valueB;


  if (valueB == NULL)
  {
    return 1;
  }
  if (valueA == chapterLink->filename) //TODO ???
  {
    return 0;
  }

  return 2;
}

void linkChapters(Vector *chapterList, Vector *linkVec, int (*compare)(void*,void*))
{
  for(int counter = 0; counter < chapterList->size; counter++)
  {
    Chapter *currentChapter = chapterList->elements[counter];
    ChapterLink *linkA = vecGet(linkVec, currentChapter->choiceA_, compare);
    ChapterLink *linkB = vecGet(linkVec, currentChapter->choiceB_, compare);
    currentChapter->choiceA_ = linkA->chapter;
    currentChapter->choiceB_ = linkB->chapter;
  }

};

void loadAllChapters(char *filename,Vector *chapterList, Vector *linkVec, int (*compare)(void*, void*))
{
  Chapter *firstChapter = NULL;
  loadChapter(filename, &firstChapter);

  ChapterLink *firstChapterLink = malloc(sizeof(ChapterLink));
  firstChapterLink->filename = filename;

  vecAdd(linkVec, firstChapterLink);
  vecAdd(chapterList, firstChapter);

  for(int counter = 0; counter < chapterList->size; counter++)
  {

    Chapter *nextChapter = chapterList->elements[counter];

    //Load A
    if (vecGet(linkVec, nextChapter->choiceA_, compare) == NULL)
    {
      ChapterLink *chapterLink = malloc(sizeof(ChapterLink));
      chapterLink->filename = nextChapter->choiceA_;

      Chapter *chapter = NULL;
      loadChapter(nextChapter->choiceA_, &chapter);
      chapterLink->chapter = chapter;

      vecAdd(linkVec, chapterLink);
      vecAdd(chapterList, chapter);
    }

    //Load B
    if (vecGet(linkVec, nextChapter->choiceB_, compare) == NULL)
    {
      ChapterLink *chapterLink = malloc(sizeof(ChapterLink));
      chapterLink->filename = nextChapter->choiceB_;

      Chapter *chapter = NULL;
      loadChapter(nextChapter->choiceB_, &chapter);
      chapterLink->chapter = chapter;

      vecAdd(linkVec, chapterLink);
      vecAdd(chapterList, chapter);
    }

  }
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

int vecTrim(Vector *vector)
{
  void *save = vector->elements;

  vector->capacity = vector->size;
  vector->elements = realloc(vector->elements, vector->capacity * sizeof(void*));
  if(vector->elements == NULL)
  {
    free(save);
    printf("Out of Memory");
    return OUT_OF_MEMORY;
  }

  return OK;
}

int vecEnsureCap(Vector *vector)
{
  if (vector->size == vector->capacity)
  {
    vector->capacity *= 2;

    void *save = vector->elements;
    vector->elements = realloc(vector->elements, vector->capacity * sizeof(void*));
    if(vector->elements == NULL)
    {
      vector->elements = save;
      PRINT_OUT_OF_MEMORY();
      return OUT_OF_MEMORY;
    }
  }
  return OK;
}

Vector *vecCreate(int initCapacity)
{
  Vector *vector = malloc(sizeof(Vector));
  if(vector == NULL)
  {
    PRINT_OUT_OF_MEMORY();
    return OUT_OF_MEMORY;
  }
  vector->capacity = initCapacity;
  vector->elements = malloc(vector->capacity * sizeof(void*));
  vector->size = 0;

  return vector;
}

int vecAdd(Vector *vector, void *newEntry)
{
  int result = vecEnsureCap(vector);
  if(FAILED(result))
  {
    return result;
  }

  vector->elements[vector->size] = newEntry;
  vector->size++;

  return OK;
}

int vecAddIndex(Vector *vector, void *newEntry, int index)
{
  int result = vecEnsureCap(vector);
  if(FAILED(result))
  {
    return result;
  }

  vector->size++;
  for(int counter = vector->size; counter > index; counter--)
  {
    vector->elements[counter] = vector->elements[counter - 1];
  }
  vector->elements[index] = newEntry;

  return OK;
}

int vecAddOrdered(Vector *vector, void *newEntry, int (*compare)(void*, void*))
{
  int result;

  for (int counter = 0; counter < vector->size; counter++)
  {
    if(compare(newEntry, vector->elements[counter]) > 0)
    {
      result = vecAddIndex(vector, newEntry, counter);
      if(FAILED(result))
      {
        return result;
      }
    }
  }

  result = vecAdd(vector, newEntry);
  if(FAILED(result))
  {
    return result;
  }

  return OK;
}

void *vecGet(Vector *vector, void *compareValue, int (*compare)(void*, void*))
{

  if(vector->size == 0)
  {
    return NULL;
  }

  for (int counter = 0; counter < vector->size; counter++)
  {
    if(compare(compareValue, vector->elements[counter]) == 0)
    {
      return vector->elements[counter];
    }
  }

  return NULL;
}

void vecDelete(Vector *vector, void (*delete)(void*))
{
  if(vector->elements)
  {
    for (int counter = 0; counter < vector->size; counter++)
    {
      delete(vector->elements[counter]);
    }
    free(vector->elements);
  }

  free(vector);
}