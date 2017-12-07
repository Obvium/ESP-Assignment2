
//-----------------------------------------------------------------------------
// ass2.c
//
// This is a system to output pre-written textadventures.
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

// Macros for printing error messages
#define PRINT_USAGE() printf("Usage: ./ass2 [file-name]\n")
#define PRINT_OUT_OF_MEMORY() printf("[ERR] Out of memory.\n")
#define PRINT_INVALID_INPUT() printf("[ERR] Please enter A or B.\n")
#define PRINT_INVALID_FILE(fn) printf("[ERR] Could not read file %s.\n", (fn))
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

// Checks if p is a null pointer before freeing it.
void safeFree(void *p);

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

// Reads a filename from an open file.
int readFileName(FILE *p_file, char **file_name_out);

// loads a single chapter into a chapter-struct
int loadChapter(FILE* p_file, Chapter **out_chapter);

// Checks if the file name chapterChoice has been red before.
int checkChapterChoice(Vector *v_chapter_links, char **chapter_choice);

// loads everything out of an opend text file (chapters and filenames)
int loadFile(Vector *v_chapters, Vector *v_chapter_links, char **file_name);

//loads all chapters of a textadventure
int loadChapters(Vector *v_chapters, Vector *v_chapter_links, char *file_name);

//links every chapter with its following 2
void linkChapters(Vector *v_chapters, Vector *v_chapter_links);

// Compares two Chapters and deletes the Chapter at indexB if they are equal.
void compareChapters(Vector *v_chapters, int index_a, int index_b);

// prints out a Chapter
int printChapter(Chapter *p_chapter);

// Returns a users input choice
char getChoice();


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
    return WRONG_USAGE;
  }

  int result;

  // creates the vector that will store chapterlinks
  Vector *v_chapter_links = NULL;
  result = vecCreate(&v_chapter_links, INITIAL_VECTOR_SIZE);
  if (FAILED(result))
  {
    if (result == OUT_OF_MEMORY)
    {
      PRINT_OUT_OF_MEMORY();
    }

    return result;
  }

  // creates the vector that will store every added chapter
  Vector *v_chapters = NULL;
  result = vecCreate(&v_chapters, INITIAL_VECTOR_SIZE);
  if (FAILED(result))
  {
    if (result == OUT_OF_MEMORY)
    {
      PRINT_OUT_OF_MEMORY();
    }
    vecDelete(v_chapter_links, freeChapterLink);

    return result;
  }

  // loads every chapter starting with the first additional commandline argument
  result = loadChapters(v_chapters, v_chapter_links, argv[1]);
  if (FAILED(result))
  {
    if (result == OUT_OF_MEMORY)
    {
      PRINT_OUT_OF_MEMORY();
    }
    vecDelete(v_chapter_links, freeChapterLink);
    vecDelete(v_chapters, freeChapter);

    return result;
  }

  // after the last chapter is loaded they need to be linked to be easily
  // accessed while playing the game
  linkChapters(v_chapters, v_chapter_links);

  // now the Chapterlink Vector is no longer needed
  vecDelete(v_chapter_links, freeChapterLink);
  v_chapter_links = NULL;

  // takes the first element of the chapter vector (it is the first file)
  char choice;
  Chapter *p_chapter = v_chapters->elements_[0];

  // prints a chapter, waits for the user to input his choice and prints the next
  // one until an endchapter is reached.
  while (SUCCESS(printChapter(p_chapter)))
  {
    choice = getChoice();

    if (choice == FAIL)
    {
      break;
    }

    if (choice == 'A')
    {
      p_chapter = p_chapter->choice_a_;
    }
    else
    {
      p_chapter = p_chapter->choice_b_;
    }
  }

  vecDelete(v_chapters, freeChapter);

  return 0;
}

//-----------------------------------------------------------------------------
///
/// Checks if p is a null pointer before freeing it.
///
/// @param p pointer to free
//
void safeFree(void *p)
{
  if(p)
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
void freeChapterLink(void *p_void)
{
  if (!p_void)
  {
    return;
  }
  ChapterLink *p_chapter_link = (ChapterLink *) p_void;
  safeFree(p_chapter_link->file_name_);
  safeFree(p_chapter_link);
}

//-----------------------------------------------------------------------------
///
/// Frees the memory allocated to a Chapter title, text and the Chapter itself.
///
/// @param pVoid a pointer to a Chapter
//
void freeChapter(void *p_void)
{
  if (!p_void)
  {
    return;
  }
  Chapter *p_chapter = (Chapter *) p_void;
  safeFree(p_chapter->title_);
  safeFree(p_chapter->text_);
  safeFree(p_chapter);
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
int compareChapterLink(const void *p_void_a, const void *p_void_b)
{
  char *file_name_a = ((ChapterLink *) p_void_a)->file_name_;
  char *file_name_b = ((ChapterLink *) p_void_b)->file_name_;

  int i;
  // compares all characters of the file names
  // for the length of the shortest one
  for (i = 0; file_name_a[i] != '\0' && file_name_b[i] != '\0'; ++i)
  {
    if (file_name_a[i] < file_name_b[i])
    {
      return -1;
    }
    else if (file_name_a[i] > file_name_b[i])
    {
      return 1;
    }
  }

  // compares the last character of one file name to the character at equal
  // index of the other file name
  return (file_name_a[i] < file_name_b[i] ?
          -1 : (file_name_a[i] != file_name_b[i]));
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
int vecCreate(Vector **vector, int init_capacity)
{
  (*vector) = malloc(sizeof(Vector));
  if (!(*vector))
  {
    return OUT_OF_MEMORY;
  }

  (*vector)->elements_ = malloc(sizeof(void *) * init_capacity);
  if (!(*vector)->elements_)
  {
    safeFree(*vector);
    return OUT_OF_MEMORY;
  }

  (*vector)->capacity_ = init_capacity;
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
    if (!vector->elements_)
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

  int low_index = 0;
  int high_index = vector->size_ - 1;
  int mid_index, cmp;

  // if the given element is "greater" than the last element
  if (compare(element, vector->elements_[high_index]) > 0)
  {
    return vecAdd(vector, element);
  }

  // if the given element is "smaller" than the fist element
  if (compare(element, vector->elements_[low_index]) < 0)
  {
    return vecAddAtIndex(vector, element, 0);
  }

  // binary search (half-interval search)
  while ((mid_index = (high_index + low_index) / 2) != low_index)
  {
    cmp = compare(element, vector->elements_[mid_index]);

    if (cmp < 0)
    {
      high_index = mid_index;
    }
    else if (cmp > 0)
    {
      low_index = mid_index;
    }
    else
    {
      return vecAddAtIndex(vector, element, mid_index);
    }

  }

  return vecAddAtIndex(vector, element, low_index + 1);
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

  int low_index = 0;
  int high_index = vector->size_ - 1;
  int mid_index, cmp;

  // if the given element is equals the first element
  cmp = compare(element, vector->elements_[low_index]);
  if (cmp == 0)
  {
    return vector->elements_[low_index];
  }
  else if (cmp < 0)
  {
    return NULL;
  }

  // if the given element is equals the last element
  cmp = compare(element, vector->elements_[high_index]);
  if (cmp == 0)
  {
    return vector->elements_[high_index];
  }
  else if (cmp > 0)
  {
    return NULL;
  }

  // binary search (half-interval search)
  while ((mid_index = (high_index + low_index) / 2) != low_index)
  {
    cmp = compare(element, vector->elements_[mid_index]);

    if (cmp < 0)
    {
      high_index = mid_index;
    }
    else if (cmp > 0)
    {
      low_index = mid_index;
    }
    else
    {
      return vector->elements_[mid_index];
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
  void *new_elements = realloc(vector->elements_,
                              vector->size_ * sizeof(void *));

  if (new_elements)
  {
    vector->elements_ = new_elements;
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
int readLine(FILE *p_file, char **out_line)
{
  (*out_line) = malloc(LINE_BUFFER_SIZE * sizeof(char));
  if (!(*out_line))
  {
    return OUT_OF_MEMORY;
  }

  int character;
  int length = 0;
  int index = 0;
  char buffer[LINE_BUFFER_SIZE];

  // stores the pointer for the case that the reallocation fails
  char *save = *out_line;

  while ((character = getc(p_file)) != '\n' && character != EOF)
  {

    // copy the values of the buffer to *outLine if the buffer is full
    if (index == LINE_BUFFER_SIZE)
    {
      (*out_line) = realloc(*out_line,
                           (length + LINE_BUFFER_SIZE) * sizeof(char));
      if (!(*out_line))
      {
        safeFree(save);
        return OUT_OF_MEMORY;
      }
      save = *out_line;

      memcpy((*out_line) + length - index,
             buffer, LINE_BUFFER_SIZE * sizeof(char));
      index = 0;
    }

    buffer[index] = (char) character;
    ++index;
    ++length;
  }

  if (character == EOF)
  {
    safeFree(*out_line);
    return INVALID_FILE;
  }

  // copy the values of the buffer to *outLine
  if (index > 0)
  {
    memcpy((*out_line) + length - index, buffer, (size_t) index);
  }

  ++length;
  (*out_line) = realloc(*out_line, length * sizeof(char)); // trim *outLine
  if (!(*out_line))
  {
    safeFree(save);
    return OUT_OF_MEMORY;
  }

  (*out_line)[length - 1] = '\0';

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
int readFileName(FILE *p_file, char **file_name_out)
{
  int result = readLine(p_file, file_name_out);
  if (FAILED(result))
  {
    return result;
  }

  // If the string equals "-", then there is no file name
  if ((*file_name_out)[0] == '-' && (*file_name_out)[1] == '\0')
  {
    safeFree(*file_name_out);
    (*file_name_out) = NULL;
  }
/* Test System doesn't know the function realpath:
 * warning: implicit declaration of function ‘realpath’
  else
  {
    // get the absolute path from the relative path fileNameOut if possible
    char* path = realpath(*file_name_out, NULL);
    if (path)
    {
      safeFree(*file_name_out);
      (*file_name_out) = path;
    }
  }
*/

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
int loadChapter(FILE* p_file, Chapter **out_chapter)
{
  int result;

  // read the title
  result = readLine(p_file, &((*out_chapter)->title_));
  if (FAILED(result))
  {
    return result;
  }

  // read the file name for choice A
  result = readFileName(p_file, (char **) &((*out_chapter)->choice_a_));
  if (FAILED(result))
  {
    return result;
  }

  // read the file name for choice B
  result = readFileName(p_file, (char **) &((*out_chapter)->choice_b_));
  if (FAILED(result))
  {
    return result;
  }

  // checks if both choices are either ends or not ends
  if ( ( (*out_chapter)->choice_a_ == NULL) !=
       ( (*out_chapter)->choice_b_ == NULL) )
  {
    return INVALID_FILE;
  }

  // get the current position in the file
  long length = ftell(p_file);

  // move to the end of the file
  if (fseek(p_file, 0, SEEK_END))
  {
    return INVALID_FILE;
  }

  // get the position at the end of the file and calculate textLength
  long text_length = ftell(p_file) - length;
  if (text_length == -1L)
  {
    return INVALID_FILE;
  }

  // move back to the original position to continue reading
  if (fseek(p_file, length, SEEK_SET))
  {
    return INVALID_FILE;
  }

  (*out_chapter)->text_ = malloc((text_length + 1) * sizeof(char));
  if (!(*out_chapter)->text_)
  {
    return OUT_OF_MEMORY;
  }

  // read the the remaining text
  size_t bytes_red = fread((*out_chapter)->text_,
                           (size_t) 1, (size_t) text_length, p_file);
  (*out_chapter)->text_[text_length] = '\0';

  if (bytes_red != text_length)
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
int checkChapterChoice(Vector *v_chapter_links, char **chapter_choice)
{
  ChapterLink chapter_link = { *chapter_choice, NULL };

  // search for a specific ChapterLink
  ChapterLink *p_chapter_link = vecGet(v_chapter_links,
                                     compareChapterLink, &chapter_link);

  if (p_chapter_link)
  {
    safeFree(*chapter_choice);
    (*chapter_choice) = p_chapter_link->file_name_;
  }
  else
  {
    p_chapter_link = malloc(sizeof(ChapterLink));
    if (!p_chapter_link)
    {
      return OUT_OF_MEMORY;
    }

    p_chapter_link->p_chapter = NULL;
    p_chapter_link->file_name_ = (*chapter_choice);
    vecInsert(v_chapter_links, compareChapterLink, p_chapter_link);
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
int loadFile(Vector *v_chapters, Vector *v_chapter_links, char **file_name)
{
  int result;

  ChapterLink chapter_link;
  chapter_link.file_name_ = *file_name;
  chapter_link.p_chapter = NULL;

  // search for a specific ChapterLink
  ChapterLink *p_chapter_link = vecGet(v_chapter_links,
                                     compareChapterLink, &chapter_link);

  if (!p_chapter_link->p_chapter)
  {

    FILE *p_file = fopen(chapter_link.file_name_, "r");
    if (!p_file)
    {
      PRINT_INVALID_FILE(chapter_link.file_name_);
      return INVALID_FILE;
    }

    // creates a new empty Chapter
    Chapter *p_chapter = malloc(sizeof(Chapter));
    if (!p_chapter)
    {
      fclose(p_file);
      return OUT_OF_MEMORY;
    }
    p_chapter->has_end_ = 0;
    p_chapter->title_ = NULL;
    p_chapter->text_ = NULL;
    p_chapter->choice_a_ = NULL;
    p_chapter->choice_b_ = NULL;

    // saves everything into the chapter
    result = loadChapter(p_file, &p_chapter);
    fclose(p_file);

    if (FAILED(result))
    {
      safeFree(p_chapter->choice_b_);
      safeFree(p_chapter->choice_a_);
      safeFree(p_chapter->title_);
      safeFree(p_chapter->text_);
      safeFree(p_chapter);

      if(result == INVALID_FILE)
      {
        PRINT_INVALID_FILE(chapter_link.file_name_);
      }

      return result;
    }

    //adds the chapter to the chapter list
    p_chapter_link->p_chapter = p_chapter;
    vecAdd(v_chapters, p_chapter);

    // Checks the file names of the choices
    // sets hasEnd to 1 if the file names are NULL
    if (p_chapter->choice_a_)
    {
      result = checkChapterChoice(v_chapter_links, (char **) &(p_chapter->choice_a_));
      if (FAILED(result))
      {
        safeFree(p_chapter->choice_a_);
        safeFree(p_chapter->choice_b_);
        return result;
      }

      result = checkChapterChoice(v_chapter_links, (char **) &(p_chapter->choice_b_));
      if (FAILED(result))
      {
        safeFree(p_chapter->choice_b_);
        return result;
      }
    }
    else
    {
      p_chapter->has_end_ = 1;
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
int loadChapters(Vector *v_chapters, Vector *v_chapter_links, char *file_name)
{
  // starts with the first one, given with the commandline argument
  char *first_file = malloc((strlen(file_name) + 1) * sizeof(char));
  if (!first_file)
  {
    return OUT_OF_MEMORY;
  }
  strcpy(first_file, file_name);

  int result;

  // creates the chapterlink Vector and adds the first file
  ChapterLink *p_chapter_link = malloc(sizeof(ChapterLink));
  if (!p_chapter_link)
  {
    safeFree(first_file);
    return OUT_OF_MEMORY;
  }
  p_chapter_link->p_chapter = NULL;
  p_chapter_link->file_name_ = first_file;
  vecAdd(v_chapter_links, p_chapter_link);

  Chapter *p_chapter;
  result = loadFile(v_chapters, v_chapter_links, &first_file);
  if (FAILED(result))
  {
    return result;
  }

  // now it starts working through the Chapter vector
  // the Chapter vector will grow until all endfiles are reached
  for (int i = 0; i < v_chapters->size_; ++i)
  {
    p_chapter = v_chapters->elements_[i];

    if (!p_chapter->choice_a_)
    {
      continue;
    }

    result = loadFile(v_chapters, v_chapter_links, (char **) &(p_chapter->choice_a_));
    if (FAILED(result))
    {
      return result;
    }

    result = loadFile(v_chapters, v_chapter_links, (char **) &(p_chapter->choice_b_));
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
void compareChapters(Vector *v_chapters, int index_a, int index_b)
{
  Chapter *p_chapter_a = v_chapters->elements_[index_a];
  Chapter *p_chapter_b = v_chapters->elements_[index_b];

  if(strcmp(p_chapter_a->title_, p_chapter_b->title_) == 0 &&
     strcmp(p_chapter_a->text_ , p_chapter_b->text_ ) == 0 &&
      p_chapter_a->choice_a_ == p_chapter_b->choice_a_ &&
      p_chapter_a->choice_b_ == p_chapter_b->choice_b_)
  {
    v_chapters->elements_[index_b] = NULL;

    Chapter *p_chapter;

    for(int k = 0; k < v_chapters->size_; ++k)
    {
      p_chapter = v_chapters->elements_[k];
      if(!p_chapter)
      {
        continue;
      }

      if(p_chapter->choice_a_ == p_chapter_b)
      {
        p_chapter->choice_a_ = p_chapter_a;
      }

      if(p_chapter->choice_b_ == p_chapter_b)
      {
        p_chapter->choice_b_ = p_chapter_a;
      }
    }

    freeChapter(p_chapter_b);
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
void linkChapters(Vector *v_chapters, Vector *v_chapter_links)
{
  ChapterLink chapter_link;
  chapter_link.p_chapter = NULL;
  ChapterLink *p_chapter_link = &chapter_link;
  Chapter *p_chapter;

  for (int i = 0; i < v_chapters->size_; ++i)
  {
    p_chapter = (Chapter *) v_chapters->elements_[i];

    if (p_chapter->choice_a_)
    {
      chapter_link.file_name_ = p_chapter->choice_a_;

      // find the the Chapter by its file name
      p_chapter->choice_a_ = ((ChapterLink *) vecGet(v_chapter_links, compareChapterLink, p_chapter_link))->p_chapter;
    }

    if (p_chapter->choice_b_)
    {
      chapter_link.file_name_ = p_chapter->choice_b_;

      // find the the Chapter by its file name
      p_chapter->choice_b_ = ((ChapterLink *) vecGet(v_chapter_links, compareChapterLink, p_chapter_link))->p_chapter;
    }
  }

  for(int i = 0; i < v_chapters->size_; ++i)
  {
    if(!v_chapters->elements_[i])
    {
      continue;
    }

    for(int j = i + 1; j < v_chapters->size_; ++j)
    {
      if(v_chapters->elements_[j])
      {
        compareChapters(v_chapters, i, j);
      }
    }
  }

  int move_count = 0;

  // remove all null pointers from the Chapter Vector
  for(int i = 0; i < v_chapters->size_; ++i)
  {
    if(!v_chapters->elements_[i])
    {
      ++move_count;
    }
    else if(move_count > 0)
    {
      v_chapters->elements_[i - move_count] = v_chapters->elements_[i];
    }
  }

  v_chapters->size_ -= move_count;
  vecTrim(v_chapters);

  int update = 1;
  // set hasEnd for every Chapter and continue as long as there is a change
  while(update)
  {
    update = 0;

    for(int i = 0; i < v_chapters->size_; ++i)
    {
      p_chapter = (Chapter *) v_chapters->elements_[i];

      if(!p_chapter->has_end_ && (
          ((Chapter *)p_chapter->choice_a_)->has_end_ ||
          ((Chapter *)p_chapter->choice_b_)->has_end_ ) )
      {
        p_chapter->has_end_ = 1;
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
int printChapter(Chapter *p_chapter)
{
  if(!p_chapter->has_end_)
  {
    PRINT_NO_END();
  }
  printf("------------------------------\n"
             "%s\n"
             "\n"
             "%s\n"
             "\n",
         p_chapter->title_, p_chapter->text_);

  if (!p_chapter->choice_a_)
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
