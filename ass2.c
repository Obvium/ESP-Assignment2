
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

// Macros for checking null pointers
#define NULLPTR(p) ((p) == NULL)
#define NOT_NULLPTR(p) ((p) != NULL)

// Macros for printing error messages
#define PRINT_USAGE() printf("Usage: ./ass2 [file-name]\n")
#define PRINT_OUT_OF_MEMORY() printf("[ERR] Out of memory.\n")
#define PRINT_INVALID_INPUT() printf("[ERR] Please enter A or B.\n")
#define PRINT_INVALID_FILE(fileName) printf("[ERR] Could not read file %s.\n", (fileName)) // TODO
#define PRINT_NO_END() printf("[INFO] You are in a circle with no end.\n")

#define LINE_BUFFER_SIZE 128
#define INITIAL_VECTOR_SIZE 64


// Contains all the important information of one text file (chapter)
typedef struct _Chapter_
{
  char has_end_;
  char *title_;
  char *text_;
  void *choice_a_;
  void *choice_b_;
} Chapter;

// Contains the pointer of a chapter to its corresponding filename
typedef struct _ChapterLink_
{
  char *file_name_;
  Chapter *p_chapter;
} ChapterLink;

// Vector as a "dynamic array"
typedef struct _Vector_
{
  int size_; // number of elements in the array
  int capacity_; // current maximum array size
  void **elements_; // array for elements
} Vector;


// Creates a new empty vector
int vecCreate(Vector **vector, int init_capacity);

// Deletes a Vector
void vecDelete(Vector *vector, void (*freeElement)(void *));

// Reallocates memory to vector elements if needed
int vecEnsureCapacity(Vector *vector);

// Adds an element at the end of an existing vector
int vecAdd(Vector *vector, void *element);

// Adds an element at the given index of an existing vector
int vecAddAtIndex(Vector *vector, void *element, int index);

// Sorts an element into an existing vector in a specific order
int vecInsert(Vector *vector, int (*compare)(const void *, const void *),
              void *element);

// Returns a specific element of the vector elements
void *vecGet(Vector *vector, int (*compare)(const void *, const void *),
             const void *element);

// Trims the vector to its size (so no unnecessary memory is allocated)
void vecTrim(Vector *vector);

// Frees the memory allocated to the file name and the ChapterLink itself.
void freeChapterLink(void *p_void);

// Frees the memory allocated to a Chapter title, text and the Chapter itself
void freeChapter(void *p_void);

// Compares two ChapterLinks by its file name
int compareChapterLink(const void *p_void_a, const void *p_void_b);

// reads a single line of a single text file
int readLine(FILE *p_file, char **out_line);

// loads a single chapter into a chapter-struct
int loadChapter(FILE* p_file, Chapter **out_chapter);

// TODO COMMENT ?
int checkChapterChoice(Vector *v_chapter_links, char **chapter_choice);

// loads everything out of an opend text file (chapters and filenames)
int loadFile(Vector *vChapters, Vector *vChapterLinks, char **fileName);

//loads all chapters of a textadventure
int loadChapters(Vector *vChapters, Vector *vChapterLinks, char *fileName);

//links every chapter with its following 2
void linkChapters(Vector *vChapters, Vector *vChapterLinks);

int printChapter(Chapter *pChapter);

// Returns a users input choice
char getChoice();


void safeFree(void *p)
{
  if(NOT_NULLPTR(p))
  {
    free(p);
  }
}

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
  safeFree(pChapterLink->file_name_);
  safeFree(pChapterLink);
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
  safeFree(pChapter->title_);
  safeFree(pChapter->text_);
  safeFree(pChapter);
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
  char *a = ((ChapterLink *) pVoidA)->file_name_;
  char *b = ((ChapterLink *) pVoidB)->file_name_;

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
  (*vector) = malloc(sizeof(Vector));
  if (NULLPTR(*vector))
  {
    return OUT_OF_MEMORY;
  }

  (*vector)->elements_ = malloc(sizeof(void *) * initCapacity);
  if (NULLPTR((*vector)->elements_))
  {
    safeFree(*vector);
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
  if (vector)
  {
    if (vector->elements_)
    {
      for (int i = 0; i < vector->size_; ++i)
      {
        freeElement(vector->elements_[i]);
      }
      safeFree(vector->elements_);
    }
    safeFree(vector);
  }
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

    vector->elements_ = realloc(vector->elements_,
                                vector->capacity_ * sizeof(void *));
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
int vecInsert(Vector *vector, int (*compare)(const void *, const void *),
              void *element)
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

  // if the given element is "smaller" than the fist element
  if (compare(element, vector->elements_[lowIndex]) < 0)
  {
    return vecAddAtIndex(vector, element, 0);
  }

  // binary search (half-interval search)
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
void *vecGet(Vector *vector, int (*compare)(const void *, const void *),
             const void *element)
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

  // binary search (half-interval search)
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
  void *newElements = realloc(vector->elements_,
                              vector->size_ * sizeof(void *));

  if (NOT_NULLPTR(newElements))
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
  (*outLine) = malloc(LINE_BUFFER_SIZE * sizeof(char));
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
      (*outLine) = realloc(*outLine,
                           (length + LINE_BUFFER_SIZE) * sizeof(char));
      if (NULLPTR(*outLine))
      {
        safeFree(save);
        return OUT_OF_MEMORY;
      }
      save = *outLine;

      memcpy((*outLine) + length - index,
             buffer, LINE_BUFFER_SIZE * sizeof(char));
      index = 0;
    }

    buffer[index] = (char) character;
    ++index;
    ++length;
  }

  if (character == EOF)
  {
    safeFree(*outLine);
    return INVALID_FILE;
  }

  // copy the values of the buffer to *outLine
  if (index > 0)
  {
    memcpy((*outLine) + length - index, buffer, (size_t) index);
  }

  ++length;
  (*outLine) = realloc(*outLine, length * sizeof(char)); // trim *outLine
  if (NULLPTR(*outLine))
  {
    safeFree(save);
    return OUT_OF_MEMORY;
  }

  (*outLine)[length - 1] = '\0';

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Reads a filename from an open file
/// and checks if it was an end-chapter
///
/// @param pFile The already opened file
/// @param fileNameOut The file name output of this function
///                    NULL if it was an end-chapter
/// @return result of readLine, if failed, or
///                OK
//
int readFileName(FILE *pFile, char **fileNameOut)
{
  int result = readLine(pFile, fileNameOut);
  if (FAILED(result))
  {
    return result;
  }

  // If the string equals "-", then there is no file name
  if ((*fileNameOut)[0] == '-' && (*fileNameOut)[1] == '\0')
  {
    safeFree(*fileNameOut);
    (*fileNameOut) = NULL;
  }
  else
  {
    // get the absolute path from the relative path fileNameOut if possible
    char* path = realpath(*fileNameOut, NULL);
    if (NOT_NULLPTR(path))
    {
      safeFree(*fileNameOut);
      (*fileNameOut) = path;
    }
  }

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Loads a whole chapter into a Chapter. Reads the first line (title) with
/// readLine, the next two (file name choice A/B) with readFileName and the
/// remaining text (text) with fread.
///
/// @param fileName The file name of the chapter it wants to read
/// @param outChapter Chapter for returning the data red
/// @return result of readLine, if failed, or
///         result of readFileName, if failed, or
///         INVALID_FILE or
///         OK
//
int loadChapter(FILE* pFile, Chapter **outChapter)
{
  int result;

  // read the title
  result = readLine(pFile, &((*outChapter)->title_));
  if (FAILED(result))
  {
    return result;
  }

  // read the file name for choice A
  result = readFileName(pFile, (char **) &((*outChapter)->choice_a_));
  if (FAILED(result))
  {
    return result;
  }

  // read the file name for choice B
  result = readFileName(pFile, (char **) &((*outChapter)->choice_b_));
  if (FAILED(result))
  {
    return result;
  }

  // checks if both choices are either ends or not ends
  if (NULLPTR((*outChapter)->choice_a_) != NULLPTR((*outChapter)->choice_b_))
  {
    return INVALID_FILE;
  }

  // get the current position in the file
  long length = ftell(pFile);

  // move to the end of the file
  if (fseek(pFile, 0, SEEK_END))
  {
    return INVALID_FILE;
  }

  // get the position at the end of the file and calculate textLength
  long textLength = ftell(pFile) - length;
  if (textLength == -1L)
  {
    return INVALID_FILE;
  }

  // move back to the original position to continue reading
  if (fseek(pFile, length, SEEK_SET))
  {
    return INVALID_FILE;
  }

  (*outChapter)->text_ = malloc((textLength + 1) * sizeof(char));
  if (NULLPTR((*outChapter)->text_))
  {
    return OUT_OF_MEMORY;
  }

  // read the the remaining text
  size_t bred = fread((*outChapter)->text_,
                      (size_t) 1, (size_t) textLength, pFile);
  (*outChapter)->text_[textLength] = '\0';

  if (bred != textLength)
  {
    return INVALID_FILE;
  }

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Checks if the file name chapterChoice has been red before. A new
/// ChapterLink is added to the vector of not.
///
/// @param vChapterLinks The ChapterLink vector
/// @param chapterChoice file name of a choice
/// @return OUT_OF_MEMORY
///         OK
//
int checkChapterChoice(Vector *vChapterLinks, char **chapterChoice)
{
  ChapterLink chapterLink = { *chapterChoice, NULL };

  // search for a specific ChapterLink
  ChapterLink *pChapterLink = vecGet(vChapterLinks,
                                     compareChapterLink, &chapterLink);

  if (pChapterLink)
  {
    safeFree(*chapterChoice);
    (*chapterChoice) = pChapterLink->file_name_;
  }
  else
  {
    pChapterLink = malloc(sizeof(ChapterLink));
    if (NULLPTR(pChapterLink))
    {
      return OUT_OF_MEMORY;
    }

    pChapterLink->p_chapter = NULL;
    pChapterLink->file_name_ = (*chapterChoice);
    vecInsert(vChapterLinks, compareChapterLink, pChapterLink);
  }

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Creates a new chapter and saves the title, the filename for choice A and B
/// and the text into it.
/// Also adds an entry inside the vector of all chapters and sets everything.
///
/// @param vChapters The vector with all chapters
/// @param vChapterLinks The vector with all chapterlinks
/// @param fileName The filename of the file you want to load
/// @return INVALID_FILE
///         OK
///         OUT_OF_MEMORY
//
int loadFile(Vector *vChapters, Vector *vChapterLinks, char **fileName)
{
  int result;

  ChapterLink chapterLink;
  chapterLink.file_name_ = *fileName;
  chapterLink.p_chapter = NULL;

  // search for a specific ChapterLink
  ChapterLink *pChapterLink = vecGet(vChapterLinks,
                                     compareChapterLink, &chapterLink);

  if (NULLPTR(pChapterLink->p_chapter))
  {

    FILE *pFile = fopen(chapterLink.file_name_, "r");
    if (!pFile)
    {
      PRINT_INVALID_FILE(chapterLink.file_name_);
      return INVALID_FILE;
    }

    // creates a new empty Chapter
    Chapter *pChapter = malloc(sizeof(Chapter));
    if (NULLPTR(pChapter))
    {
      fclose(pFile);
      return OUT_OF_MEMORY;
    }
    pChapter->has_end_ = 0;
    pChapter->title_ = NULL;
    pChapter->text_ = NULL;
    pChapter->choice_a_ = NULL;
    pChapter->choice_b_ = NULL;

    // saves everything into the chapter
    result = loadChapter(pFile, &pChapter);
    fclose(pFile);

    if (FAILED(result))
    {
      safeFree(pChapter->choice_b_);
      safeFree(pChapter->choice_a_);
      safeFree(pChapter->title_);
      safeFree(pChapter->text_);
      safeFree(pChapter);

      if(result == INVALID_FILE)
      {
        PRINT_INVALID_FILE(chapterLink.file_name_);
      }

      return result;
    }

    //adds the chapter to the chapter list
    pChapterLink->p_chapter = pChapter;
    vecAdd(vChapters, pChapter);

    // Checks the file names of the choices
    // sets hasEnd to 1 if the file names are NULL
    if (NOT_NULLPTR(pChapter->choice_a_))
    {
      result = checkChapterChoice(vChapterLinks, (char **) &(pChapter->choice_a_));
      if (FAILED(result))
      {
        safeFree(pChapter->choice_a_);
        safeFree(pChapter->choice_b_);
        return result;
      }

      result = checkChapterChoice(vChapterLinks, (char **) &(pChapter->choice_b_));
      if (FAILED(result))
      {
        safeFree(pChapter->choice_b_);
        return result;
      }
    }
    else
    {
      pChapter->has_end_ = 1;
    }
  }

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Reads all chapters. Starting from the file with the name fileName and
/// reading all files written in subfiles as file choices.
///
/// @param vChapters Vector for storing the Chapters
/// @param vChapterLinks Vector for storing the ChapterLinks
/// @param fileName The filename of the first file
/// @return result of loadFile or
///         OUT_OF_MEMORY or
///         OK
//
int loadChapters(Vector *vChapters, Vector *vChapterLinks, char *fileName)
{
  // starts with the first one, given with the commandline argument
  char *firstFile = malloc((strlen(fileName) + 1) * sizeof(char));
  if (NULLPTR(firstFile))
  {
    return OUT_OF_MEMORY;
  }
  strcpy(firstFile, fileName);

  int result;

  // creates the chapterlink Vector and adds the first file
  ChapterLink *pChapterLink = malloc(sizeof(ChapterLink));
  if (NULLPTR(pChapterLink))
  {
    safeFree(firstFile);
    return OUT_OF_MEMORY;
  }
  pChapterLink->p_chapter = NULL;
  pChapterLink->file_name_ = firstFile;
  vecAdd(vChapterLinks, pChapterLink);

  Chapter *pChapter;
  result = loadFile(vChapters, vChapterLinks, &firstFile);
  if (FAILED(result))
  {
    return result;
  }

  // now it starts working through the Chapter vector
  // the Chapter vector will grow until all endfiles are reached
  for (int i = 0; i < vChapters->size_; ++i)
  {
    pChapter = vChapters->elements_[i];

    if (NULLPTR(pChapter->choice_a_))
    {
      continue;
    }

    result = loadFile(vChapters, vChapterLinks, (char **) &(pChapter->choice_a_));
    if (FAILED(result))
    {
      return result;
    }

    result = loadFile(vChapters, vChapterLinks, (char **) &(pChapter->choice_b_));
    if (FAILED(result))
    {
      return result;
    }

  }

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Compares two Chapters and deletes the Chapter at indexB if they are equal.
/// Also replaces all usages of the pointer to ChapterB by the pointer to
/// ChapterA.
///
/// @param vChapters Vector of Chapters
/// @param indexA index of the first Chapter to compare
/// @param indexB index of the second Chapter to compare
//
void compareChapters(Vector *vChapters, int indexA, int indexB)
{
  Chapter *pChapterA = vChapters->elements_[indexA];
  Chapter *pChapterB = vChapters->elements_[indexB];

  if(strcmp(pChapterA->title_, pChapterB->title_) == 0 &&
     strcmp(pChapterA->text_ , pChapterB->text_ ) == 0 &&
     pChapterA->choice_a_ == pChapterB->choice_a_ &&
     pChapterA->choice_b_ == pChapterB->choice_b_)
  {
    vChapters->elements_[indexB] = NULL;

    Chapter *pChapter;

    for(int k = 0; k < vChapters->size_; ++k)
    {
      pChapter = vChapters->elements_[k];
      if(NULLPTR(pChapter))
      {
        continue;
      }

      if(pChapter->choice_a_ == pChapterB)
      {
        pChapter->choice_a_ = pChapterA;
      }

      if(pChapter->choice_b_ == pChapterB)
      {
        pChapter->choice_b_ = pChapterA;
      }
    }

    freeChapter(pChapterB);
  }
}

//-----------------------------------------------------------------------------
///
/// Replaces the file names for the choices stored in the Chapters by pointer
/// to the corresponding Chapter.
/// Removes all copies of any Chapter and Trims the Chapter Vector.
/// Sets the hasEnd variable for all Chapters.
///
/// @param vChapters The vector with every chapter
/// @param vChapterLinks The vector with all the linked chapter-pointers
//
void linkChapters(Vector *vChapters, Vector *vChapterLinks)
{
  ChapterLink chapterLink;
  chapterLink.p_chapter = NULL;
  ChapterLink *pChapterLink = &chapterLink;
  Chapter *pChapter;

  for (int i = 0; i < vChapters->size_; ++i)
  {
    pChapter = (Chapter *) vChapters->elements_[i];

    if (NOT_NULLPTR(pChapter->choice_a_))
    {
      chapterLink.file_name_ = pChapter->choice_a_;

      // find the the Chapter by its file name
      pChapter->choice_a_ = ((ChapterLink *) vecGet(vChapterLinks, compareChapterLink, pChapterLink))->p_chapter;
    }

    if (NOT_NULLPTR(pChapter->choice_b_))
    {
      chapterLink.file_name_ = pChapter->choice_b_;

      // find the the Chapter by its file name
      pChapter->choice_b_ = ((ChapterLink *) vecGet(vChapterLinks, compareChapterLink, pChapterLink))->p_chapter;
    }
  }

  for(int i = 0; i < vChapters->size_; ++i)
  {
    if(NULLPTR(vChapters->elements_[i]))
    {
      continue;
    }

    for(int j = i + 1; j < vChapters->size_; ++j)
    {
      if(NOT_NULLPTR(vChapters->elements_[j]))
      {
        compareChapters(vChapters, i, j);
      }
    }
  }

  int moveCount = 0;

  // remove all null pointers from the Chapter Vector
  for(int i = 0; i < vChapters->size_; ++i)
  {
    if(NULLPTR(vChapters->elements_[i]))
    {
      ++moveCount;
    }
    else if(moveCount > 0)
    {
      vChapters->elements_[i - moveCount] = vChapters->elements_[i];
    }
  }

  vChapters->size_ -= moveCount;
  vecTrim(vChapters);

  int update = 1;
  // set hasEnd for every Chapter and continue as long as there is a change
  while(update)
  {
    update = 0;

    for(int i = 0; i < vChapters->size_; ++i)
    {
      pChapter = (Chapter *) vChapters->elements_[i];

      if(!pChapter->has_end_ && (
          ((Chapter *)pChapter->choice_a_)->has_end_ ||
          ((Chapter *)pChapter->choice_b_)->has_end_ ) )
      {
        pChapter->has_end_ = 1;
        update = 1;
      }
    }
  }
}

//-----------------------------------------------------------------------------
///
/// Prints the title and the text of an specific chapter.
/// Optinally it also prints if the player can't reach an end.
///
/// @param pChapter The pointer to the chapter
/// @return if the the textadventure reached one of its endings or not
//
int printChapter(Chapter *pChapter)
{
  if(!pChapter->has_end_)
  {
    PRINT_NO_END();
  }
  printf("------------------------------\n"
             "%s\n"
             "\n"
             "%s\n"
             "\n",
         pChapter->title_, pChapter->text_);

  if (NULLPTR(pChapter->choice_a_))
  {
    printf("ENDE\n");
    return END;
  }
  printf("Deine Wahl (A/B)? ");

  return OK;
}

//-----------------------------------------------------------------------------
///
/// Waits for the user to input either A, B or EOF and returns the result
///
/// @return 'A' or
///         'B' or
///         FAIL
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
///
/// The program starts here.
/// First it reads all files and stores them in Chapters and ChapterLinks.
/// When finished, all the Chapters are "linked" togethere so that each Chapter
/// points to two other Chapters or NULL if it is an end.
/// Starts the Game after that, printing a Chapter after every choice the user
/// is asked to make.
///
/// @param argc Count of commandline arguments
/// @param argv The commandline arguments
/// @return WRONG_USAGE if no filename was given
///         OUT_OF_MEMORY
///         INVALID_FILE
///         OK
//
int main(int argc, char **argv)
{

  // only one additional argument possible/needed
  if (argc != 2)
  {
    PRINT_USAGE();
    printf("ret = %i\n", WRONG_USAGE);
    return WRONG_USAGE;
  }

  int result;

  // creates the vector that will store chapterlinks
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

  // creates the vector that will store every added chapter
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

  // loads every chapter starting with the first additional commandline argument
  result = loadChapters(vChapters, vChapterLinks, argv[1]);
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

  // after the last chapter is loaded they need to be linked to be easily
  // accessed while playing the game
  linkChapters(vChapters, vChapterLinks);

  // now the Chapterlink Vector is no longer needed
  vecDelete(vChapterLinks, freeChapterLink);
  vChapterLinks = NULL;

  // takes the first element of the chapter vector (it is the first file)
  char choice;
  Chapter *pChapter = vChapters->elements_[0];

  // prints a chapter, waits for the user to input his choice and prints the next
  // one until an endchapter is reached.
  while (SUCCESS(printChapter(pChapter)))
  {
    choice = getChoice();

    if (choice == FAIL)
    {
      break;
    }

    if (choice == 'A')
    {
      pChapter = pChapter->choice_a_;
    }
    else
    {
      pChapter = pChapter->choice_b_;
    }
  }

  vecDelete(vChapters, freeChapter);

  printf("ret = 0\n");
  return 0;
}