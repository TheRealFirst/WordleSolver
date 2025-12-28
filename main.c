#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define WORDS 12972
#define TESTRUNS 1000


int rand_lim(int limit) {
    int divisor = RAND_MAX/(limit+1);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}



int main(void) {
    static char letters[26];
    int testResults[TESTRUNS] = {0};
    int multipleGuessAmount[TESTRUNS] = {0};
    srand((unsigned)time(NULL));  

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

    for(int tests = 0; tests < TESTRUNS; tests++)
    {
        int wordIndex = rand_lim(WORDS - 1);

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

        int guesses = 0;

        for(int x = 0; x < 6; x++)
        {    
            guesses++;
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

            

            char input[6];

            if (size == 1) {
                memcpy(input, words[candidates[index[0]]], 6);
            } else {
                multipleGuessAmount[tests]++;
                memcpy(input, words[candidates[index[rand_lim(size-1)]]], 6);
            }


            char validation[5] = {0};
            
            int remaining[26] = {0};

            for (int i = 0; i < 5; i++) {
                remaining[words[wordIndex][i] - 'a']++;
                validation[i] = '0';
            }

            for (int i = 0; i < 5; i++)
            {
                if(input[i] == words[wordIndex][i])
                {
                    validation[i] = '2';
                    remaining[input[i] - 'a']--;
                }
            }

            for(int i = 0; i < 5; i++)
            {
                if(validation[i] == '2') continue;
                int idx = input[i] - 'a';
                if(remaining[idx] > 0)
                {
                    validation[i] = '1';
                    remaining[idx]--;
                }
            }


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
            if(candidatesCount == 0)
            {
                break;
                guesses = 0;
            }
                

            printf("Remaining possibilites: %i \n", candidatesCount);

            free(index);
        }

        testResults[tests] = guesses;
    }

    FILE *out = fopen("tests/results.csv", "w");
    if (!out) {
        perror("fopen results.csv");
        return 1;
    }

    fprintf(out, "test,guesses,amount_of_multiguess\n");

    for (int i = 0; i < TESTRUNS; i++) {
        fprintf(out, "%d,%d,%d\n", i, testResults[i], multipleGuessAmount[i]);
    }

    fclose(out);


    return 0;
}
