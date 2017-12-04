
//-----------------------------------------------------------------------------
// ass2.c
//
// This is a system to output prewritten textadventures.
// It saves the whole story at its start.
//
//
// Group: Group 2, study assistant Florian Hager
//
// Authors:
// Stefan Fragner 11703708
// Tobias Topar 11710538
//-----------------------------------------------------------------------------
//

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>


// Return codes
#define FAIL 0
#define OK 10
#define WRONG_USAGE 1
#define OUT_OF_MEMORY 2
#define INVALID_FILE 3
#define END FAIL

// Macros for checking return codes
#define FAILED(r) ((r) != OK)
#define SUCCESS(r) ((r) == OK)
#define NULLPTR(p) ((p) == NULL)

// Macros for printing error messages
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

// Contains all the important information of one text file (chapter)
typedef struct _Chapter_
{
  char *title_;
  char *text_;
  void *choiceA_;
  void *choiceB_;
} Chapter;

// Contains the pointer of a chapter to its corresponding filename
typedef struct _ChapterLink_
{
  char *fileName_;
  Chapter *pChapter_;
} ChapterLink;

// Vector for a "Vector-system"
typedef struct _Vector_
{
  int size_; // number of elements in the array
  int capacity_; // current maximum array size
  void **elements_; // array for elements
} Vector;


// Creates a new empty vector
int vecCreate(Vector **vector, int initCapacity);

// Deletes a Vector
void vecDelete(Vector *vector, void (*freeElement)(void *));

// Reallocates memory to vector elements if needed
int vecEnsureCapacity(Vector *vector);

// Adds an element at the end of an existing vector
int vecAdd(Vector *vector, void *element);

// Adds an element at the given index of an existing vector
int vecAddAtIndex(Vector *vector, void *element, int index);

// Sorts an element into an existing vector in a specific order
int vecInsert(Vector *vector, int (*compare)(const void *, const void *), void *element);

// Returns a specific element of the vector elements
void *vecGet(Vector *vector, int (*compare)(const void *, const void *), const void *element);

// Trims the vector to its size (so no unnecessary memory is allocated)
void vecTrim(Vector *vector);

void freeChapterLink(void *pVoid);
void freeChapter(void *pVoid);

// Compares two ChapterLinks by its fileName_
int compareChapterLink(const void *av, const void *bv);

// reads a single line of a single text file
int readLine(FILE *pFile, char **outLine);

// TODO COMMENT ?
int loadChapter(char *fileName, Chapter **outChapter);

// TODO COMMENT ?
int checkFileChoice(Vector *vChapterLinks, char **fileChoice);

// TODO COMMENT ?
int loadFile(Vector *vChapters, Vector *vChapterLinks, char **fileName);

int loadChapters(Vector *vChapters, Vector *vChapterLinks, char *fileName);

void linkChapters(Vector *vChapters, Vector *vChapterLinks);

int printChapter(Chapter *pChapter);

// Returns a users input choice
char getChoice();

//-----------------------------------------------------------------------------
///
/// Frees the memory allocated to the file name in a ChapterLink and the
/// ChapterLink itself.
///
/// @param pVoid a pointer to a ChapterLink
//
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

//-----------------------------------------------------------------------------
///
/// Frees the memory allocated to a Chapter title, text and the Chapter itself.
///
/// @param pVoid a pointer to a Chapter
//
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

//-----------------------------------------------------------------------------
///
/// Compares the fileNames of two different ChapterLinks
///
/// @param pVoidA void pointer to element A
/// @param pVoidB void pointer to element B
///
/// @return -1 if the file name of pVoidA is lexicographically smaller than the
///            file name of pVoidB
///          0 if the file name of pVoidA is equal to the file name of pVoidB
///          1 if the file name of pVoidA is lexicographically bigger than the
///            file name of pVoidB
///
//
int compareChapterLink(const void *pVoidA, const void *pVoidB)
{
  char *a = ((ChapterLink *) pVoidA)->fileName_;
  char *b = ((ChapterLink *) pVoidB)->fileName_;

  int i;
  // compares all characters of the file names
  // for the length of the shortest one
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

  // compares the last character of one file name to the character at equal
  // index of the other file name
  return (a[i] < b[i] ? -1 : (a[i] != b[i]));
}

//-----------------------------------------------------------------------------
///
/// Allocates memory to the Vector and the elements of a Vector with a capacity
/// of initCapacity.
///
/// @param vector A predefined vector
/// @param initCapacity
/// @return OUT_OF_MEMORY or
///         OK
//
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

//-----------------------------------------------------------------------------
///
/// Frees every element of a vector with freeElement, vector->elements and the
/// vector itself.
///
/// @param vector Vector you want to be freed
/// @param freeElement Function to free the elements of the vector
//
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

//-----------------------------------------------------------------------------
///
/// Checks if there is enough memory allocated to add ONE more element to a
/// vector. If not, it reallocates twice as much as the current capacity.
///
/// @param vector
/// @return OUT_OF_MEMORY or
///         OK
//
int vecEnsureCapacity(Vector *vector)
{
  if (vector->size_ == vector->capacity_)
  {
    vector->capacity_ <<= 1;

    // stores the pointer for the case that the reallocation fails
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

//-----------------------------------------------------------------------------
///
/// Adds the element to the end of the elements of a vector if there is enough
/// memory.
///
/// @param vector
/// @param element The element to add
/// @return result of vecEnsureCapacity if it fails or
///         OK
//
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

//-----------------------------------------------------------------------------
///
/// Adds the element at the given index to the elements of a vector if there is
/// enough memory.
///
/// @param vector
/// @param element The element to add
/// @param index Index where the element should be added
/// @return result of vecEnsureCapacity if it fails or
///         OK
//
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

  // moves every element after index to the next position
  while (start != end)
  {
    *start = *(start - 1);
    --start;
  }

  vector->elements_[index] = element;
  ++vector->size_;

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Adds the given element at the correct index determined by the given
/// compare function.
///
/// @param vector
/// @param compare The function to compare the given element with already
///                existing ones, to find the right location
/// @param element The element to add
/// @return result of vecAdd or
///         result of vecAddAtIndex
//
int vecInsert(Vector *vector, int (*compare)(const void *, const void *), void *element)
{
  if (vector->size_ == 0)
  {
    return vecAdd(vector, element);
  }

  int lowIndex = 0;
  int highIndex = vector->size_ - 1;
  int midIndex, cmp;

  // if the given element is "greater" than the last element
  if (compare(element, vector->elements_[highIndex]) > 0)
  {
    return vecAdd(vector, element);
  }

  // if the given element is "smaller" than the fist element // TODO research
  if (compare(element, vector->elements_[lowIndex]) < 0)
  {
    return vecAddAtIndex(vector, element, 0);
  }

  // TODO research name?
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

//-----------------------------------------------------------------------------
///
/// Searches for an element in the elements of a Vector determined by the
/// given compare function and the given search criteria element.
///
/// @param vector
/// @compare The function to compare the given element with already
///                existing ones, to find the right location
/// @param element The search criteria
/// @return The element if found or
///         NULL if not found
//
void *vecGet(Vector *vector, int (*compare)(const void *, const void *), const void *element)
{
  if (vector->size_ == 0)
  {
    return NULL;
  }

  int lowIndex = 0;
  int highIndex = vector->size_ - 1;
  int midIndex, cmp;

  // if the given element is equals the first element
  cmp = compare(element, vector->elements_[lowIndex]);
  if (cmp == 0)
  {
    return vector->elements_[lowIndex];
  }
  else if (cmp < 0)
  {
    return NULL;
  }

  // if the given element is equals the last element
  cmp = compare(element, vector->elements_[highIndex]);
  if (cmp == 0)
  {
    return vector->elements_[highIndex];
  }
  else if (cmp > 0)
  {
    return NULL;
  }

  // TODO RESEARCH name?
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

//-----------------------------------------------------------------------------
///
/// Trims the given vector down to its size. So no excess-memory is allocated.
///
/// @param vector
//
void vecTrim(Vector *vector)
{
  if (vector->size_ == vector->capacity_)
  {
    return;
  }

  // allocate to a new variable in the case that the reallocation fails
  void *newElements = REALLOC(vector->elements_, vector->size_ * sizeof(void *));

  if (!NULLPTR(newElements))
  {
    vector->elements_ = newElements;
    vector->capacity_ = vector->size_;
  }
}

//-----------------------------------------------------------------------------
///
/// Reads a single line of an open file and returns it via outLine.
///
/// @param pFile The already opened file
/// @param outLine The line output
/// @return OUT_OF_MEMORY
///         INVALID_FILE
///         OK
//
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

  // stores the pointer for the case that the reallocation fails
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

    // copy the values of the buffer to *outLine if the buffer is full
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

  // copy the values of the buffer to *outLine
  if (index > 0)
  {
    memcpy((*outLine) + length - index, buffer, (size_t) index);
  }

  ++length;
  (*outLine) = REALLOC(*outLine, length * sizeof(char)); // trim *outLine
  if (NULLPTR(*outLine))
  {
    FREE(save);
    return OUT_OF_MEMORY;
  }

  (*outLine)[length - 1] = '\0';

  return OK;
}

//-----------------------------------------------------------------------------
/// TODO COMMENT
/// Used to read the filename insed the opend txt file.
///
///
/// @param pFile The already opened file
/// @param fileNameOut The output of this function (NULL if it was an end-chapter
/// @return if there was enough space allocated to read the file
//
int readFileName(FILE *pFile, char **fileNameOut)
{
  int result = readLine(pFile, fileNameOut);
  if (FAILED(result))
  {
    return result;
  }
// TODO implement
  if ((*fileNameOut)[0] == '-' && (*fileNameOut)[1] == '\0')
  {
    FREE(*fileNameOut);
    (*fileNameOut) = NULL;
  }

  return OK;
}

//-----------------------------------------------------------------------------
/// TODO COMMENT
/// Loads a whole chapter into the a chapter struct
///
/// @param fileName The filename of the Chapter.txt it wants to read
/// @param outChapter Outputs the read chapter
/// @return if an error occured or not
//
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

  //creates a new empty Chapter
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

  //at first it tries to read the title of the Chapter
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

  //reads the Filename for choice A inside the current Chapter
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

  //reads the Filename for choice B inside the current Chapter
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

  //TODO
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

//-----------------------------------------------------------------------------
///
///
///
/// @param vChapterLinks
/// @param chapterChoice
/// @return
//
int checkFileChoice(Vector *vChapterLinks, char **chapterChoice) // TODO rename fileChoice ?
{
  ChapterLink chapterLink = { *chapterChoice, NULL };
  ChapterLink *pChapterLink = vecGet(vChapterLinks, compareChapterLink, &chapterLink);

  if (pChapterLink)
  {
    FREE(*chapterChoice);
    (*chapterChoice) = pChapterLink->fileName_;
  }
  else
  {
    pChapterLink = MALLOC(sizeof(ChapterLink));
    if (NULLPTR(pChapterLink))
    {
      return OUT_OF_MEMORY;
    }

    pChapterLink->pChapter_ = NULL;
    pChapterLink->fileName_ = (*chapterChoice);
    vecInsert(vChapterLinks, compareChapterLink, pChapterLink);
  }

  return OK;
}

//-----------------------------------------------------------------------------
/// //TODO
/// @param vChapters
/// @param vChapterLinks
/// @param fileName
/// @return
//
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

//-----------------------------------------------------------------------------
/// Reads every chapter till it got every end-chapter
///
/// @param vChapters Vector of all chapters
/// @param vChapterLinks Vector of all linked chapter-pointers
/// @param fileName The filename of the first file
/// @return if any errors occured
//
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

//-----------------------------------------------------------------------------
/// Opens up a chapter and takes the filename for the next chapter-choice
/// and replaces it with the pointer to the next chapter
///
/// @param vChapters The vector with every chapter
/// @param vChapterLinks The vector with all the linked chapter-pointers
//
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

//-----------------------------------------------------------------------------
/// Prints the title and the text of an specific chapter
///
/// @param pChapter The pointer to the chapter
/// @return of the the textadventure reached one of its endings or not
//
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

//-----------------------------------------------------------------------------
/// waits for the user to input either A or B and returns the result
///
/// @return A or B
//
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

//-----------------------------------------------------------------------------
/// Combines all functions and prints out all possible error-messages if necessary
/// it also requires an TODO argument containing the first textfile of a textadventur
///
/// @param argc //TODO (name)
/// @param argv
/// @return
//
int main(int argc, char **argv)
{
#ifdef DEBUG // TODO REMOVE
  argc = 2;
#endif

  // only one additional argument possible/needed
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

  //after the last chapter is loaded they need to be linked to be easily accessed while playing the game
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
