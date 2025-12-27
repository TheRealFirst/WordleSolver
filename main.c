#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define WORDS 12972


int main(void) {
    static char letters[26];

    for(int i = 0; i < 26; i++)
        letters[i] = 'a' + i;

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

    int stats[26];

    int positionStat[5][26];


    int score[WORDS];

    char correctLetters[5] = {0};
    int maxCount[26];
    int minCount[26] = {0};
    uint8_t forbidPos[5][26] = {0};

    for(int i = 0; i < 26; i++)
    {
        maxCount[i] = 5;
    }
    

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

        for(int i = 0; i < candidatesCount; i++)
        {
            for(int j = 0; j < 5; j++)
            {
                for(int k = 0; k < 26; k++)
                {
                    if(words[candidates[i]][j] == letters[k])
                    {
                        stats[k]++;
                        positionStat[j][k]++;
                        break;
                    }
                }
            } 
        }

        memset(score, 0, sizeof score);
        
        for(int i = 0; i < candidatesCount; i++)
        {
            for(int j = 0; j < 5; j++)
            {
                for(int k = 0; k < 26; k++)
                {
                    if(words[candidates[i]][j] == letters[k])
                    {
                        score[i] += positionStat[j][k];
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

        if (size == 1) {
            memcpy(input, words[candidates[index[0]]], 6);
            printf("Guessed (auto): %s\n", input);
        } else {
            printf("Guessed: ");
            scanf("%5s", input);
        }


        char validation[6];

        printf("for each Position (no spaces) tell me if it was correct or wrong (not in word = 0, in word = 1, correct place = 2)");
        scanf("%5s", validation);

        int solved = 1;
        for (int i = 0; i < 5; i++) {
            if (validation[i] != '2') {
                solved = 0;
                break;
            }
        }

        if (solved) {
            printf("Solved!\n");
            break;
        }

        int guessMin[26] = {0};
        int guessGray[26] = {0};

        for (int pos = 0; pos < 5; ++pos) {
            int digit = validation[pos] - '0';
            int letter = input[pos] - 'a';
            switch (digit)
            {
            case 0:
                guessGray[letter]++;
                break;
            case 1:
                forbidPos[pos][letter] = 1;
                guessMin[letter]++;
                break;
            case 2:
                correctLetters[pos] = input[pos];
                guessMin[letter]++;
                break;
            default:
                break;
            }
        }

        for(int i = 0; i < 26; i++)
        {
            if(guessMin[i] == 0 && guessGray[i] > 0)
            {
                maxCount[i] = 0;
            }
            else if(guessMin[i] > 0 && guessGray[i] > 0)
            {
                maxCount[i] = guessMin[i];
            }

            if(minCount[i] < guessMin[i]) minCount[i] = guessMin[i];
        }

        int write = 0;
        for(int i = 0; i < candidatesCount; i++)
        {
            int idx = candidates[i];
            char *w = words[idx];
            
            int ok = 1;
            for(int pos = 0; pos < 5; pos++)
            {
                if(correctLetters[pos] && w[pos] != correctLetters[pos]) {ok = 0; break;}
            }
            if(!ok) continue;

            for(int pos = 0; pos < 5; pos++)
            {
                int letter = w[pos] - 'a';
                if(forbidPos[pos][letter]) {ok = 0; break;}
            }
            if(!ok) continue;

            int count[26] = {0};

            for(int pos = 0; pos < 5; pos++) count[w[pos] - 'a']++;

            for(int j = 0; j < 26; j++)
            {
                if(maxCount[j] == 0 && count[j] > 0) {ok = 0; break;}
                if(count[j] < minCount[j]) {ok = 0; break;}
                if(count[j] > maxCount[j]) {ok = 0; break;}
            }
            if(!ok) continue;

            candidates[write++] = idx;
        }

        candidatesCount = write;
        printf("Remaining possibilites: %i \n", candidatesCount);

        free(index);
    }

    return 0;
}
