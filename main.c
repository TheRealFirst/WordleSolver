#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define WORDS 12972

typedef struct{
        char letter;
        uint16_t count;
    }LetterStat;

static void swap(LetterStat *a, LetterStat *b) {
    LetterStat tmp = *a;
    *a = *b;
    *b = tmp;
}

static int partition(LetterStat arr[], int low, int high) {
    // pivot: first element
    uint32_t pcount = arr[low].count;
    char pletter = arr[low].letter;

    int i = low;
    int j = high;

    while (i < j) {
        // We want DESCENDING by count:
        // move i forward while arr[i] should stay on the left side
        while (i <= high - 1 &&
               (arr[i].count > pcount ||
               (arr[i].count == pcount && arr[i].letter <= pletter))) {
            i++;
        }

        // move j backward while arr[j] should stay on the right side
        while (j >= low + 1 &&
               (arr[j].count < pcount ||
               (arr[j].count == pcount && arr[j].letter > pletter))) {
            j--;
        }

        if (i < j) swap(&arr[i], &arr[j]);
    }

    swap(&arr[low], &arr[j]);
    return j;
}

static void quickSort(LetterStat arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void remove_letter(LetterStat arr[], int *size, char letter) {
    for (int i = 0; i < *size; i++) {
        if (arr[i].letter == letter) {
            for (int j = i; j < *size - 1; j++) {
                arr[j] = arr[j + 1];
            }
            (*size)--;
            return;
        }
    }
}

int main(void) {
    FILE *fp = fopen("data/wordle.csv", "rb");
    if (!fp) { perror("fopen"); return 1; }

    if (fseek(fp, 0, SEEK_END) != 0) { perror("fseek"); fclose(fp); return 1; }
    long fs = ftell(fp);
    if (fs <= 0) { fprintf(stderr, "empty/bad file\n"); fclose(fp); return 1; }
    rewind(fp);

    size_t file_size = (size_t)fs;
    char *buffer = (char*)malloc(file_size + 1);
    if (!buffer) { perror("malloc"); fclose(fp); return 1; }

    size_t nread = fread(buffer, 1, file_size, fp);
    fclose(fp);
    buffer[nread] = '\0';

    char *p   = buffer;
    char *end = buffer + nread;

    while (p < end && *p != '\n') ++p;
    if (p < end) ++p;
    if (p < end && *p == '\r') ++p;    

    static char words[WORDS][6];

    for (int i = 0; i < WORDS; ++i) {
        words[i][0] = p[0];
        words[i][1] = p[1];
        words[i][2] = p[2];
        words[i][3] = p[3];
        words[i][4] = p[4];
        words[i][5] = '\0';

        while (p < end && *p != '\n') ++p;
        if (p < end) ++p;
        if (p < end && *p == '\r') ++p; 
    }

    free(buffer);

    LetterStat stats[26];

    LetterStat positionStat[5][26];


    int score[WORDS];
    int sizes[5] = {26, 26, 26, 26, 26};

    char correctLetters[5] = {0};
    char wrongLetters[21] = {0};
    int wrongLetterCount = 0;

    int candidates[WORDS];
    int candidatesCount = WORDS;
    
    for(int i = 0; i < WORDS; i++)
    {
        candidates[i] = i;
    }

    for(int x = 0; x < 6; x++)
    {    
        memset(stats, 0, sizeof stats);
        memset(positionStat, 0, sizeof positionStat);

        for (int i = 0; i < 26; i++) {
            stats[i].letter = 'a' + i;
            for (int j = 0; j < 5; j++) {
                positionStat[j][i].letter = 'a' + i;
            }
        }

        for(int i = 0; i < candidatesCount; i++)
        {
            for(int j = 0; j < 5; j++)
            {
                for(int k = 0; k < 26; k++)
                {
                    if(words[candidates[i]][j] == stats[k].letter)
                    {
                        stats[k].count += 1;
                        positionStat[j][k].count += 1;
                        break;
                    }
                }
            } 
        }

        
        int n = (int)(sizeof(stats) / sizeof(stats[0]));

        quickSort(stats, 0, n - 1);

        for(int i = 0; i < 5; i++)
        {
            quickSort(positionStat[i], 0, 25);
        }


        memset(score, 0, sizeof score);
        
        for(int i = 0; i < candidatesCount; i++)
        {
            for(int j = 0; j < 5; j++)
            {
                for(int k = 0; k < sizes[j]; k++)
                {
                    if(words[candidates[i]][j] == positionStat[j][k].letter)
                    {
                        score[i] += 26 - k;
                        break;
                    }
                }
            }
        }

        int size = 1;
        int max = 0;
        int* index;

        index = (int*)calloc(size, sizeof(int));

        if(index == NULL)
        {
            printf("Failed allocating memory");
            return 1;
        }

        index[0] = 0;

        for(int i = 0; i < candidatesCount; i++)
        {
            if(score[i] > max){
                size = 1;
                index = realloc(index, size * sizeof(int));
                index[0] = i;
                max = score[i];
            }
            else if(score[i] == max)
            {
                size++;
                index = realloc(index, size * sizeof(int));
                index[size - 1] = i;
            }
        }

        printf("good guessing words: score %i \n", max);

        for(int i = 0; i < size; i++)
        {
            printf("%s \n", words[candidates[index[i]]]);
        }

        char input[6];

        printf("Guessed: ");
        scanf("%5s", input);

        char validation[6];

        printf("for each Position (no spaces) tell me if it was correct or wrong (not in word = 0, in word = 1, correct place = 2)");
        scanf("%5s", validation);


        for (int pos = 0; pos < 5; ++pos) {
            int digit = validation[pos] - '0';
            switch (digit)
            {
            case 0:
                for(int i = 0; i < 5; i++)
                {
                    remove_letter(positionStat[i], &sizes[i], input[pos]);
                }
                wrongLetters[wrongLetterCount] = input[pos];
                wrongLetterCount++;
                break;
            case 1:
                remove_letter(positionStat[pos], &sizes[pos], input[pos]);
                break;
            case 2:
                for(int i = 0; i < sizes[pos]; i++)
                {   
                    if(positionStat[pos][i].letter == input[pos])
                    {
                        positionStat[pos][0] = positionStat[pos][i];
                        sizes[pos] = 1;
                        correctLetters[pos] = input[pos];
                        break;
                    }
                }
                break;
            default:
                break;
            }
        }

        for (int pos = 0; pos < 5; ++pos) {
            quickSort(positionStat[pos], 0, sizes[pos] - 1);
        }
        
        int i = 0;
        while (i < candidatesCount)
        {
            int removed = 0;

            for(int j = 0; j < 5; j++)
            {
                if(correctLetters[j] != 0)
                {
                    if(words[candidates[i]][j] != correctLetters[j])
                    {
                        candidates[i] = candidates[candidatesCount-1];
                        candidatesCount--;
                        removed = 1;
                        break;
                    }
                }
                
                for(int k = 0; k < wrongLetterCount; k++)
                    {
                        if(wrongLetters[k] == words[candidates[i]][j])
                        {
                            candidates[i] = candidates[candidatesCount-1];
                            candidatesCount--;
                            removed = 1;
                            break;
                        }
                    }
            }
            if(removed == 0) i++;
        }

        free(index);
    }


    return 0;
}
