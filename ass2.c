
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>


#define FAIL 0
#define OK 10
#define WRONG_USAGE 1
#define OUT_OF_MEMORY 2
#define INVALID_FILE 3
#define END FAIL

#define FAILED(r) ((r) != OK)
#define SUCCESS(r) ((r) == OK)
#define NULLPTR(p) ((p) == NULL)

#define PRINT_USAGE() printf("Usage: ./ass2 [file-name]\n")
#define PRINT_OUT_OF_MEMORY() printf("[ERR] Out of memory.\n")
#define PRINT_INVALID_INPUT() printf("[ERR] Please enter A or B.\n")
#define PRINT_INVALID_FILE(fileName) printf("[ERR] Could not read file %s.\n", (fileName))

#define LINE_BUFFER_SIZE 128
#define INITIAL_VECTOR_SIZE 64

// TODO REMOVE
// ----------------------------------------------
//#define DEBUG_MEM
#ifdef DEBUG_MEM
#define FREE mFree
#define MALLOC mMalloc
#define REALLOC mRealloc
#else
#define FREE free
#define MALLOC malloc
#define REALLOC realloc
#endif

void mFree(void *p)
{
  printf("#free %p\n", p);
  free(p);
}

void *mMalloc(size_t size)
{
  void *p = malloc(size);
  printf("#malloc %li : %p\n", size, p);
  return p;
}

void *mRealloc(void *p, size_t size)
{
  void *pn = realloc(p, size);
  printf("#realloc %li : %p --> %p\n", size, p, pn);
  return pn;
}
// ----------------------------------------------


typedef struct _Chapter_
{
  char *title_;
  char *text_;
  void *choiceA_;
  void *choiceB_;
} Chapter;


typedef struct _ChapterLink_
{
  char *fileName_;
  Chapter *pChapter_;
} ChapterLink;


typedef struct _Vector_
{
  int size_;
  int capacity_;
  void **elements_;
} Vector;


int vecCreate(Vector **vector, int initCapacity);
void vecDelete(Vector *vector, void (*freeElement)(void *));
int vecEnsureCapacity(Vector *vector);
int vecAdd(Vector *vector, void *element);
int vecAddAtIndex(Vector *vector, void *element, int index);
int vecInsert(Vector *vector, int (*compare)(const void *, const void *), void *element);
void *vecGet(Vector *vector, int (*compare)(const void *, const void *), const void *element);
void vecTrim(Vector *vector);

void freeChapterLink(void *pVoid);
void freeChapter(void *pVoid);
int compareChapterLink(const void *av, const void *bv);

int readLine(FILE *pFile, char **outLine);
int loadChapter(char *fileName, Chapter **outChapter);
int checkFileChoice(Vector *vChapterLinks, char **fileChoice);
int loadFile(Vector *vChapters, Vector *vChapterLinks, char **fileName);

int loadChapters(Vector *vChapters, Vector *vChapterLinks, char *fileName);
void linkChapters(Vector *vChapters, Vector *vChapterLinks);

int printChapter(Chapter *pChapter);
char getChoice();


void freeChapterLink(void *pVoid)
{
  if (NULLPTR(pVoid))
  {
    return;
  }
  ChapterLink *pChapterLink = (ChapterLink *) pVoid;
  FREE(pChapterLink->fileName_);
  FREE(pChapterLink);
}


void freeChapter(void *pVoid)
{
  if (NULLPTR(pVoid))
  {
    return;
  }
  Chapter *pChapter = (Chapter *) pVoid;
  FREE(pChapter->title_);
  FREE(pChapter->text_);
  FREE(pChapter);
}


int compareChapterLink(const void *av, const void *bv)
{
  char *a = ((ChapterLink *) av)->fileName_;
  char *b = ((ChapterLink *) bv)->fileName_;

  int i;
  for (i = 0; a[i] != '\0' && b[i] != '\0'; ++i)
  {
    if (a[i] < b[i])
    {
      return -1;
    }
    else if (a[i] > b[i])
    {
      return 1;
    }
  }

  return (a[i] < b[i] ? -1 : (a[i] != b[i]));
}


int vecCreate(Vector **vector, int initCapacity)
{
  (*vector) = MALLOC(sizeof(Vector));
  if (NULLPTR(*vector))
  {
    return OUT_OF_MEMORY;
  }

  (*vector)->elements_ = MALLOC(sizeof(void *) * initCapacity);
  if (NULLPTR((*vector)->elements_))
  {
    FREE(*vector);
    return OUT_OF_MEMORY;
  }

  (*vector)->capacity_ = initCapacity;
  (*vector)->size_ = 0;

  return OK;
}


void vecDelete(Vector *vector, void (*freeElement)(void *))
{
// TODO REMOVE
// ----------------------------------------------
  static int n = 0;
// ----------------------------------------------
  if (vector)
  {
    if (vector->elements_)
    {
      for (int i = 0; i < vector->size_; ++i)
      {
// TODO REMOVE
// ----------------------------------------------
#ifdef DEBUG_MEM
        if(n == 0) {
            ChapterLink* c = vector->elements[i];
            if(c->fileName_) {
                printf("+ CL[%d]: %s\n", i, c->fileName_);
            }else {
                printf("+ CL[%d]: %s\n", i, "filename = null");
            }
        }
#endif
// ----------------------------------------------
        freeElement(vector->elements_[i]);

// TODO REMOVE
// ----------------------------------------------
        printf("- CL[%d]\n", i);
// ----------------------------------------------
      }
      FREE(vector->elements_);
    }
    FREE(vector);
  }

// TODO REMOVE
// ----------------------------------------------
  n = 1;
// ----------------------------------------------
}


int vecEnsureCapacity(Vector *vector)
{
  if (vector->size_ == vector->capacity_)
  {
    vector->capacity_ <<= 1;
    void *save = vector->elements_;

    vector->elements_ = REALLOC(vector->elements_, vector->capacity_ * sizeof(void *));
    if (NULLPTR(vector->elements_))
    {
      vector->elements_ = save;
      return OUT_OF_MEMORY;
    }
  }
  return OK;
}


int vecAdd(Vector *vector, void *element)
{
  int result = vecEnsureCapacity(vector);
  if (FAILED(result))
  {
    return result;
  }

  vector->elements_[vector->size_] = element;
  ++vector->size_;

  return OK;
}


int vecAddAtIndex(Vector *vector, void *element, int index)
{
// TODO REMOVE
// ----------------------------------------------
// printf("#vecAddAtIndex cap=%i, size=%i, i=%i\n", vector->capacity, vector->size, index);
// ----------------------------------------------

  int result = vecEnsureCapacity(vector);
  if (FAILED(result))
  {
    return result;
  }

  void **start = vector->elements_ + vector->size_;
  void **end = vector->elements_ + index;

  while (start != end)
  {
    *start = *(start - 1);
    --start;
  }

  vector->elements_[index] = element;
  ++vector->size_;

  return OK;
}


int vecInsert(Vector *vector, int (*compare)(const void *, const void *), void *element)
{
  if (vector->size_ == 0)
  {
    return vecAdd(vector, element);
  }

  int lowIndex = 0;
  int highIndex = vector->size_ - 1;
  int midIndex, cmp;

  if (compare(element, vector->elements_[highIndex]) > 0)
  {
    return vecAdd(vector, element);
  }

  if (compare(element, vector->elements_[lowIndex]) < 0)
  {
    return vecAddAtIndex(vector, element, 0);
  }

  while ((midIndex = (highIndex + lowIndex) / 2) != lowIndex)
  {
    cmp = compare(element, vector->elements_[midIndex]);

    if (cmp < 0)
    {
      highIndex = midIndex;
    }
    else if (cmp > 0)
    {
      lowIndex = midIndex;
    }
    else
    {
      return vecAddAtIndex(vector, element, midIndex);
    }

  }

  return vecAddAtIndex(vector, element, lowIndex + 1);
}


void *vecGet(Vector *vector, int (*compare)(const void *, const void *), const void *element)
{
  if (vector->size_ == 0)
  {
    return NULL;
  }

  int lowIndex = 0;
  int highIndex = vector->size_ - 1;
  int midIndex, cmp;

  cmp = compare(element, vector->elements_[lowIndex]);
  if (cmp == 0)
  {
    return vector->elements_[lowIndex];
  }
  else if (cmp < 0)
  {
    return NULL;
  }

  cmp = compare(element, vector->elements_[highIndex]);
  if (cmp == 0)
  {
    return vector->elements_[highIndex];
  }
  else if (cmp > 0)
  {
    return NULL;
  }

  while ((midIndex = (highIndex + lowIndex) / 2) != lowIndex)
  {
    cmp = compare(element, vector->elements_[midIndex]);

    if (cmp < 0)
    {
      highIndex = midIndex;
    }
    else if (cmp > 0)
    {
      lowIndex = midIndex;
    }
    else
    {
      return vector->elements_[midIndex];
    }

  }

  return NULL;
}


void vecTrim(Vector *vector)
{
  if (vector->size_ == vector->capacity_)
  {
    return;
  }

  void *newElements = REALLOC(vector->elements_, vector->size_ * sizeof(void *));

  if (!NULLPTR(newElements))
  {
    vector->elements_ = newElements;
    vector->capacity_ = vector->size_;
  }
}


int readLine(FILE *pFile, char **outLine)
{
  (*outLine) = MALLOC(LINE_BUFFER_SIZE * sizeof(char));
  if (NULLPTR(*outLine))
  {
    return OUT_OF_MEMORY;
  }

  int character;
  int length = 0;
  int index = 0;
  char buffer[LINE_BUFFER_SIZE];
  char *save = *outLine;

  while ((character = getc(pFile)) != '\n' && character != EOF)
  {
// TODO REMOVE
// ----------------------------------------------
    if ((char) character == '\r')
    {
      continue;
    }
// ----------------------------------------------

    if (index == LINE_BUFFER_SIZE)
    {
      (*outLine) = REALLOC(*outLine, (length + LINE_BUFFER_SIZE) * sizeof(char));
      if (NULLPTR(*outLine))
      {
        FREE(save);
        return OUT_OF_MEMORY;
      }
      save = *outLine;

      memcpy((*outLine) + length - index, buffer, LINE_BUFFER_SIZE * sizeof(char));
      index = 0;
    }

    buffer[index] = (char) character;
    ++index;
    ++length;
  }

  if (character == EOF)
  {
    FREE(*outLine);
    return INVALID_FILE;
  }

  if (index > 0)
  {
    memcpy((*outLine) + length - index, buffer, (size_t) index);
  }

  ++length;
  (*outLine) = REALLOC(*outLine, length * sizeof(char));
  if (NULLPTR(*outLine))
  {
    FREE(save);
    return OUT_OF_MEMORY;
  }

  (*outLine)[length - 1] = '\0';

  return OK;
}


int readFileName(FILE *pFile, char **fileNameOut)
{
  int result = readLine(pFile, fileNameOut);
  if (FAILED(result))
  {
    return result;
  }

  if ((*fileNameOut)[0] == '-' && (*fileNameOut)[1] == '\0')
  {
    FREE(*fileNameOut);
    (*fileNameOut) = NULL;
  }

  return OK;
}


int loadChapter(char *fileName, Chapter **outChapter)
{
  int result;

  FILE *pFile = fopen(fileName, "r");
  if (!pFile)
  {
    PRINT_INVALID_FILE(fileName);
    return INVALID_FILE;
  }

// TODO REMOVE
// ----------------------------------------------
#ifdef DEBUG // TODO REMOVE
  char path[4096];
  strcpy(path, fileName_);
  if(realpath(fileName_, path)) {
  printf("path: %s\n", path);
  }
#endif
// ----------------------------------------------

  (*outChapter) = MALLOC(sizeof(Chapter));
  if (NULLPTR(*outChapter))
  {
    fclose(pFile);
    return OUT_OF_MEMORY;
  }
  (*outChapter)->title_ = NULL;
  (*outChapter)->text_ = NULL;
  (*outChapter)->choiceA_ = NULL;
  (*outChapter)->choiceB_ = NULL;

  result = readLine(pFile, &((*outChapter)->title_));
  if (FAILED(result))
  {
    FREE(*outChapter);
    fclose(pFile);

    if (result == INVALID_FILE)
    {
      PRINT_INVALID_FILE(fileName);
    }
    return result;
  }

  result = readFileName(pFile, (char **) &((*outChapter)->choiceA_));
  if (FAILED(result))
  {
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    if (result == INVALID_FILE)
    {
      PRINT_INVALID_FILE(fileName);
    }
    return result;
  }

  result = readFileName(pFile, (char **) &((*outChapter)->choiceB_));
  if (FAILED(result))
  {
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    if (result == INVALID_FILE)
    {
      PRINT_INVALID_FILE(fileName);
    }
    return result;
  }

  if (NULLPTR((*outChapter)->choiceA_) != NULLPTR((*outChapter)->choiceB_))
  {
    FREE((*outChapter)->choiceB_);
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    PRINT_INVALID_FILE(fileName);
    return INVALID_FILE;
  }

  long length = ftell(pFile);

  if (fseek(pFile, 0, SEEK_END))
  {
    FREE((*outChapter)->choiceB_);
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    PRINT_INVALID_FILE(fileName);
    return INVALID_FILE;
  }

  long textLength = ftell(pFile) - length;
  if (textLength == -1L)
  {
    FREE((*outChapter)->choiceB_);
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    PRINT_INVALID_FILE(fileName);
    return INVALID_FILE;
  }

  if (fseek(pFile, length, SEEK_SET))
  {
    FREE((*outChapter)->choiceB_);
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    PRINT_INVALID_FILE(fileName);
    return INVALID_FILE;
  }

  (*outChapter)->text_ = MALLOC((textLength + 1) * sizeof(char));
  if (NULLPTR((*outChapter)->text_))
  {
    FREE((*outChapter)->choiceB_);
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE(*outChapter);
    fclose(pFile);

    return OUT_OF_MEMORY;
  }

  size_t bred = fread((*outChapter)->text_, (size_t) 1, (size_t) textLength, pFile);
  (*outChapter)->text_[textLength] = '\0';

  fclose(pFile);

  if (bred != textLength)
  {
    FREE((*outChapter)->choiceB_);
    FREE((*outChapter)->choiceA_);
    FREE((*outChapter)->title_);
    FREE((*outChapter)->text_);
    FREE(*outChapter);

    PRINT_INVALID_FILE(fileName);
    return INVALID_FILE;
  }

  return OK;
}


int checkFileChoice(Vector *vChapterLinks, char **fileChoice)
{
  ChapterLink chapterLink = {*fileChoice, NULL};
  ChapterLink *pChapterLink = vecGet(vChapterLinks, compareChapterLink, &chapterLink);

  if (pChapterLink)
  {
    FREE(*fileChoice);
    (*fileChoice) = pChapterLink->fileName_;
  }
  else
  {
    pChapterLink = MALLOC(sizeof(ChapterLink));
    if (NULLPTR(pChapterLink))
    {
      return OUT_OF_MEMORY;
    }

    pChapterLink->pChapter_ = NULL;
    pChapterLink->fileName_ = (*fileChoice);
    vecInsert(vChapterLinks, compareChapterLink, pChapterLink);
  }

  return OK;
}


int loadFile(Vector *vChapters, Vector *vChapterLinks, char **fileName)
{
  int result;

  ChapterLink chapterLink;
  chapterLink.fileName_ = *fileName;
  chapterLink.pChapter_ = NULL;

  ChapterLink *pChapterLink = vecGet(vChapterLinks, compareChapterLink, &chapterLink);

// TODO REMOVE ???
// ----------------------------------------------
  if ((*fileName) != pChapterLink->fileName_)
  {
    (*fileName) = pChapterLink->fileName_;
    FREE(chapterLink.fileName_);
    printf("----- ##### loadFile() FREE fileName_ ##### -----\n");
  }
// ----------------------------------------------

  if (NULLPTR(pChapterLink->pChapter_))
  {
    Chapter *pChapter;
    result = loadChapter(chapterLink.fileName_, &pChapter);
    if (FAILED(result))
    {
      return result;
    }

    pChapterLink->pChapter_ = pChapter;
    vecAdd(vChapters, pChapter);

    if (pChapter->choiceA_)
    {
      result = checkFileChoice(vChapterLinks, (char **) &(pChapter->choiceA_));
      if (FAILED(result))
      {
        FREE(pChapter->choiceA_);
        FREE(pChapter->choiceB_);
        return result;
      }
    }

    if (pChapter->choiceB_)
    {
      result = checkFileChoice(vChapterLinks, (char **) &(pChapter->choiceB_));
      if (FAILED(result))
      {
        FREE(pChapter->choiceB_);
        return result;
      }
    }
  }

  return OK;
}


int loadChapters(Vector *vChapters, Vector *vChapterLinks, char *fileName)
{
  char *firstFile = MALLOC((strlen(fileName) + 1) * sizeof(char));
  if (NULLPTR(firstFile))
  {
    return OUT_OF_MEMORY;
  }
  strcpy(firstFile, fileName);

  int result;

  ChapterLink *pChapterLink = MALLOC(sizeof(ChapterLink));
  if (NULLPTR(pChapterLink))
  {
    FREE(firstFile);
    return OUT_OF_MEMORY;
  }
  pChapterLink->pChapter_ = NULL;
  pChapterLink->fileName_ = firstFile;
  vecAdd(vChapterLinks, pChapterLink);

  Chapter *pChapter;
  result = loadFile(vChapters, vChapterLinks, &firstFile);
  if (FAILED(result))
  {
    return result;
  }

  for (int i = 0; i < vChapters->size_; ++i)
  {
    pChapter = vChapters->elements_[i];

    if (NULLPTR(pChapter->choiceA_))
    {
      continue;
    }

    result = loadFile(vChapters, vChapterLinks, (char **) &(pChapter->choiceA_));
    if (FAILED(result))
    {
      return result;
    }

    result = loadFile(vChapters, vChapterLinks, (char **) &(pChapter->choiceB_));
    if (FAILED(result))
    {
      return result;
    }

// TODO REMOVE
// ----------------------------------------------
//if(vChapterLinks->size % 100 == 0) {
    printf("file: %d\n", vChapterLinks->size_);
    fflush(stdout);
//}
// ----------------------------------------------

  }

  vecTrim(vChapters);

  return OK;
}


void linkChapters(Vector *vChapters, Vector *vChapterLinks)
{
  ChapterLink chapterLink;
  chapterLink.pChapter_ = NULL;
  ChapterLink *pChapterLink = &chapterLink;
  Chapter *pChapter;

  for (int i = 0; i < vChapters->size_; ++i)
  {
    pChapter = (Chapter *) vChapters->elements_[i];

    if (!NULLPTR(pChapter->choiceA_))
    {
      chapterLink.fileName_ = pChapter->choiceA_;
      pChapter->choiceA_ = ((ChapterLink *) vecGet(vChapterLinks, compareChapterLink, pChapterLink))->pChapter_;
    }

    if (!NULLPTR(pChapter->choiceB_))
    {
      chapterLink.fileName_ = pChapter->choiceB_;
      pChapter->choiceB_ = ((ChapterLink *) vecGet(vChapterLinks, compareChapterLink, pChapterLink))->pChapter_;
    }
  }

}


int printChapter(Chapter *pChapter)
{
  printf("------------------------------\n"
             "%s\n"
             "\n"
             "%s\n"
             "\n",
         pChapter->title_, pChapter->text_);

  if (NULLPTR(pChapter->choiceA_))
  {
    printf("ENDE\n");
    return END;
  }
  printf("Deine Wahl (A/B)? ");

  return OK;
}


char getChoice()
{
  char choice;
  int character;

  while (1)
  {
    character = getchar();
    if (character == EOF)
    {
      return FAIL;
    }

    if (character != 10)
    {
      choice = (char) character;
      character = getchar();
      if (character == EOF)
      {
        return FAIL;
      }

      if (character == 10 && (choice == 'A' || choice == 'B'))
      {
        return choice;
      }
    }

    while (character != 10)
    {
      character = getchar();
      if (character == EOF)
      {
        return FAIL;
      }
    }

    PRINT_INVALID_INPUT();
  }
}


int main(int argc, char **argv)
{
#ifdef DEBUG // TODO REMOVE
  argc = 2;
#endif

  if (argc != 2)
  {
    PRINT_USAGE();
    printf("ret = %i\n", WRONG_USAGE);
    return WRONG_USAGE;
  }

  int result;

  Vector *vChapterLinks = NULL;
  result = vecCreate(&vChapterLinks, INITIAL_VECTOR_SIZE);
  if (FAILED(result))
  {
    if (result == OUT_OF_MEMORY)
    {
      PRINT_OUT_OF_MEMORY();
    }

    printf("ret = %i\n", result);
    return result;
  }

  Vector *vChapters = NULL;
  result = vecCreate(&vChapters, INITIAL_VECTOR_SIZE);
  if (FAILED(result))
  {
    if (result == OUT_OF_MEMORY)
    {
      PRINT_OUT_OF_MEMORY();
    }
    vecDelete(vChapterLinks, freeChapterLink);

    printf("ret = %i\n", result);
    return result;
  }

#ifdef DEBUG // TODO REMOVE
  result = loadChapters(vChapters, vChapterLinks, "test7/c1.txt");
#else
  result = loadChapters(vChapters, vChapterLinks, argv[1]);
#endif
  if (FAILED(result))
  {
    if (result == OUT_OF_MEMORY)
    {
      PRINT_OUT_OF_MEMORY();
    }
    vecDelete(vChapterLinks, freeChapterLink);
    vecDelete(vChapters, freeChapter);

    printf("ret = %i\n", result);
    return result;
  }

  linkChapters(vChapters, vChapterLinks);

  vecDelete(vChapterLinks, freeChapterLink);
  vChapterLinks = NULL;

  char choice;
  Chapter *pChapter = vChapters->elements_[0];

  while (SUCCESS(printChapter(pChapter)))
  {
    choice = getChoice();

    if (choice == FAIL)
    {
      break;
    }

    if (choice == 'A')
    {
      pChapter = pChapter->choiceA_;
    }
    else
    {
      pChapter = pChapter->choiceB_;
    }
  }

  vecDelete(vChapters, freeChapter);

  printf("ret = 0\n");
  return 0;
}
