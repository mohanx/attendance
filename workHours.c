//
//FIXME: currently only 1 year
//
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define FILENAME "attendance"//"currentWeek"
#define LINE_MAX 50
#define WEEK_MAX 52
#define WEEK_NUM 1 
#define ANSI_COLOR_RED "\033[22;31m"
#define ANSI_COLOR_LIGHTRED "\033[01;31m"
#define ANSI_COLOR_WHITERED "\033[31;47m"
#define ANSI_COLOR_YELLOWBLUE "\033[38;5;12;1m\033[48;5;11m"
#define ANSI_COLOR_WHITEBLUE "\033[34;47m"
#define ANSI_COLOR_LIGHTCYAN "\033[01;36m"
#define ANSI_COLOR_RESET "\x1b[0m"

//
// modes:
//      0 - Last week   
//      1 - specefic week 
//

enum time{HR, MIN, SEC, DAY};
enum days{Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday};

int* timeWorked(int** inTime, int** outTime, int numDays);
int* computeOneDay(int day, int* inTime, int* outTime, int* oneDay);
int* remTime(int* totalTime);
void offThursday(int* remTime, int* nowTime);
void backOnSchedule(int* rem, int* nowTime);

void curWeek(int weekCnt, int** inTime, int** outTime, struct tm* timeinfo);
void oneWeek(int weekCnt, int** inTime, int** outTime, struct tm* timeinfo);

int main(int argc, char** argv)
{
    time_t rawtime;
    time( &rawtime );
    struct tm * timeinfo;
    timeinfo = localtime ( &rawtime );
    
    printf ("Today is %s", asctime(timeinfo));
    
    FILE* fp = fopen(FILENAME, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);


    int** inTime = malloc(sizeof(int*)*7*WEEK_MAX); 
    int** outTime = malloc(sizeof(int*)*7*WEEK_MAX); 
    for(int i=0; i<(7*WEEK_MAX); i++)
    {
        inTime[i] = malloc(sizeof(int)*3);
        outTime[i] = malloc(sizeof(int)*3);
        for(int j=0; j<3; j++)
        {
            inTime[i][j] = 0;
            outTime[i][j] = 0;
        }
    }

    
    int cnt = 0;
    char day = 'X';
    char buf[LINE_MAX];
    int weekCnt = 0;
    int** week; //month, day, year
    week = malloc(sizeof(int*) * WEEK_MAX); //one year
    for(int i=0; i<WEEK_MAX; i++)
    {
        week[i] = malloc(sizeof(int*) * 4); //one year
        for(int j=0;j<4;j++)
        {
            week[i][j]=0;
        }
    }
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        // process line here
#ifdef DEBUG
        printf("scanned %s\n", buf);
#endif
        if( (buf[0]=='W') && (buf[1]=='E') ) //if first word is WEEK
        {
            if(week[51][0] == WEEK_MAX) //we've hit 52 weeks.
                week = realloc(week, sizeof(int*) * WEEK_MAX); //one year
            sscanf(buf, "WEEK%d %d-%d-%d", &week[weekCnt][0],&week[weekCnt][1], &week[weekCnt][2], &week[weekCnt][3]);  
            weekCnt++;
        }
        else{
            sscanf(buf, "%c %d:%d:%d %d:%d:%d", &day, &inTime[cnt][HR], &inTime[cnt][MIN], &inTime[cnt][SEC], &outTime[cnt][HR], &outTime[cnt][MIN], &outTime[cnt][SEC]);
#ifdef DEBUG
            printf("%c\n\tIN - %d:%d:%d \n\tOUT - %d:%d:%d\n", day, inTime[cnt][HR], inTime[cnt][MIN], inTime[cnt][SEC], outTime[cnt][HR], outTime[cnt][MIN], outTime[cnt][SEC]);
#endif
            cnt++;
        }
    }
    fclose(fp);
//    printf("cnt %d\nweeks %d\n",cnt, weekCnt);

    //NOW time
    outTime[cnt-1][HR] = timeinfo->tm_hour;
    outTime[cnt-1][MIN] = timeinfo->tm_min;
    outTime[cnt-1][SEC] = timeinfo->tm_sec;

    //
    //
    // FINISHED READING DATA
    // NOW COMPUTE 
    //
    //

    curWeek( weekCnt,  inTime,  outTime, timeinfo);
    oneWeek( WEEK_NUM,  inTime,  outTime, timeinfo);

    for(int i=0; i<(7*WEEK_MAX); i++)
    {
        free(inTime[i]);
        free(outTime[i]);
    }
    for(int i=0; i<WEEK_MAX; i++) //TODO: extend more than a year
    {
        free(week[i]);
    }
    return 0 ;
}

void curWeek(int weekCnt, int** inTime, int** outTime, struct tm* timeinfo)
{
    printf("\nCURRENT : @@ " ANSI_COLOR_LIGHTCYAN"WEEK %d" ANSI_COLOR_RESET " @@\n",weekCnt);
    int weekStart = (weekCnt*7) - 7;
    int** curInTime = malloc(sizeof(int*)*7);
    int** curOutTime = malloc(sizeof(int*)*7);
    for(int i=0; i<7; i++)
    {
        curInTime[i] = malloc(sizeof(int)*3);
        curOutTime[i] = malloc(sizeof(int)*3);
        for(int j=0; j<3; j++)
        {
            curInTime[i][j] = inTime[weekStart+i][j];
            curOutTime[i][j] = outTime[weekStart+i][j];
        }
    }
    int* t_worked;
    t_worked = timeWorked(curInTime, curOutTime, timeinfo->tm_wday);

    int* remainingTime;
    remainingTime = remTime(t_worked);

    int nowTime[4] = {0};
    nowTime[HR] = timeinfo->tm_hour;
    nowTime[MIN] = timeinfo->tm_min;
    nowTime[SEC] = timeinfo->tm_sec;
    nowTime[DAY] = timeinfo->tm_wday; // days since sunday(0) -> 6

    backOnSchedule( remainingTime, nowTime);

    free(t_worked);
    free(remainingTime);
    for(int i=0; i<7; i++)
    {
        free(curInTime[i]);
        free(curOutTime[i]);
    }
}
void oneWeek(int weekCnt, int** inTime, int** outTime, struct tm* timeinfo)
{
    printf("\n\t@@ WEEK %d @@\n",weekCnt);
    int weekStart = (weekCnt*7) - 7;
    int** curInTime = malloc(sizeof(int*)*7);
    int** curOutTime = malloc(sizeof(int*)*7);
    for(int i=0; i<7; i++)
    {
        curInTime[i] = malloc(sizeof(int)*3);
        curOutTime[i] = malloc(sizeof(int)*3);
        for(int j=0; j<3; j++)
        {
            curInTime[i][j] = inTime[weekStart+i][j];
            curOutTime[i][j] = outTime[weekStart+i][j];
        }
    }
    int* t_worked;
    t_worked = timeWorked(curInTime, curOutTime, 6);

    int* remainingTime;
    remainingTime = remTime(t_worked);

    int nowTime[4] = {0};
    nowTime[HR] = timeinfo->tm_hour;
    nowTime[MIN] = timeinfo->tm_min;
    nowTime[SEC] = timeinfo->tm_sec;
    nowTime[DAY] = timeinfo->tm_wday; // days since sunday(0) -> 6

    printf("worked %02d:%02d:%02d  out of 42:30:00\n", t_worked[HR], t_worked[MIN], t_worked[SEC]);
    printf("Rem.Time %02d:%02d:%02d\n", remainingTime[HR], remainingTime[MIN], remainingTime[SEC]);
//    offThursday(remainingTime, nowTime);

    free(t_worked);
    free(remainingTime);
    for(int i=0; i<7; i++)
    {
        free(curInTime[i]);
        free(curOutTime[i]);
    }
}

/*
 WEEKLY REMAINING HOURS  

 % get input hours %  
    read file "attendanceWeek"
 % calculate % 
    for(5)
        sum += computeOneDay;
 % print results % 
*/
int* computeOneDay(int day, int* inTime, int* outTime, int* oneDay){

    //outTime - inTime - 30min(lunch)
    oneDay[SEC] = outTime[SEC] - inTime[SEC];
    oneDay[MIN] = outTime[MIN] - inTime[MIN] - 30;
    if(day > 4 )
        oneDay[MIN]+=30;
    oneDay[HR] = outTime[HR] - inTime[HR];
    while(oneDay[SEC] < 0)
    {
        oneDay[SEC] += 60; 
        oneDay[MIN]--;
    }
    while(oneDay[MIN] < 0)
    {
        oneDay[MIN] += 60; 
        oneDay[HR]--;
    }
#ifdef DEBUG
    switch(day){
        case 0:
            printf("Sunday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        case 1:
            printf("Monday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        case 2:
            printf("Tuesday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        case 3:
            printf("Wednesday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        case 4:
            printf("Thursday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        case 5:
            printf("Friday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        case 6:
            printf("Saturday || in(%d:%d:%d) out(%d:%d:%d) =  %d:%d:%d\n", inTime[HR], inTime[MIN], inTime[SEC], outTime[HR], outTime[MIN], outTime[SEC], oneDay[HR], oneDay[MIN], oneDay[SEC]);
            break;
        default:
            printf("error: reading day\n");
            break;
    }
#endif
    return oneDay;
}       

int* timeWorked(int** inTime, int** outTime, int numDays)
{
    int* worked = malloc(sizeof(int)*3);
    int* temp = malloc(sizeof(int)*3);

    for(int i=0; i<3; i++)
        worked[i] = 0;


    for(int day=0; day <= numDays; day++)
    {
        temp = computeOneDay(day,inTime[day], outTime[day], temp);
        worked[HR]+=temp[HR];
        worked[SEC]+=temp[SEC];
        if(worked[SEC]>59)
        {
            worked[SEC] -= 60;
            worked[MIN]++;
        }
        worked[MIN]+=temp[MIN];
        if(worked[MIN]>59)
        {
            worked[MIN] -= 60;
            worked[HR]++;
        }
#ifdef debug
        printf("worked %02d:%02d:%02d\n", worked[HR], worked[MIN], worked[SEC]);
#endif
    }
    free(temp);
#ifdef debug
    printf("TOTAL WORKED %02d:%02d:%02d\n", worked[HR], worked[MIN], worked[SEC]);
#endif
    return worked;
}

int* remTime(int* t_worked){
    // remaining  = (40HRs + 2.5lunch) - t_worked
    int totalSupposed[3] = {42, 30, 0}; //42:30:00 
    int* remain = malloc(sizeof(int)*3);

    remain[SEC] = totalSupposed[SEC] - t_worked[SEC];
    remain[MIN] = totalSupposed[MIN] - t_worked[MIN];
    remain[HR]  = totalSupposed[HR] - t_worked[HR];
    if(remain[SEC] < 0)
    {
        remain[SEC] += 60; 
        remain[MIN]--;
    }
    if(remain[MIN] < 0)
    {
        remain[MIN] += 60; 
        remain[HR]--;
    }
#ifdef DEBUG
    printf("DEBUGGING: total RemTime ++ %02d:%02d:%02d ++\n", remain[HR], remain[MIN], remain[SEC]);
#endif

    return remain;
}

void offThursday(int* rem, int* nowTime){

    // AssuMINg 8:30 inTime on Thursday,
    // projecting 8HRs per remaining days and today leave at 15:30
    // Time to leave is 15:30 + remaining - 8:30*remDays - (15:30-now).
    
    int remDays = 4 - nowTime[DAY]; //remaining days in the week ex/today
    int todayRemTime[3] = {0};
    int buffTime[3] = {0};
    int leaveTime[3] = {0};

    todayRemTime[HR] = 15 - nowTime[HR];
    todayRemTime[MIN] = 30 - nowTime[MIN];
    while(todayRemTime[MIN] < 0)
    {
        todayRemTime[MIN] += 60;
        todayRemTime[HR] -= 1;
    }
#ifdef DEBUG
    printf("DEBUGGING: Today remTime ++ %02d:%02d:%02d ++\n", todayRemTime[HR], todayRemTime[MIN], todayRemTime[SEC]);
#endif

    buffTime[HR] = rem[HR] - (8*remDays) - todayRemTime[HR];
    buffTime[MIN] = rem[MIN] - todayRemTime[MIN] - (30*remDays);
    while(buffTime[MIN]<(-59))
    {
        buffTime[MIN] += 60;
        buffTime[HR]--;
    }
    if(buffTime[HR] > 0 && buffTime[MIN] < 0)
    {
        buffTime[MIN] += 60;
        buffTime[HR]--;
    }

#ifdef DEBUG
    printf("DEBUGGING: extra time ++ %02d:%02d:%02d ++\n", buffTime[HR], buffTime[MIN], buffTime[SEC]);
#endif

    leaveTime[HR] =  15 + buffTime[HR];
    leaveTime[MIN] = 30 + buffTime[MIN]; 
    while(leaveTime[MIN] < 0)
    {
        leaveTime[MIN] += 60;
        leaveTime[HR]--;
    }
    while(leaveTime[MIN] > 59)
    {
        leaveTime[MIN] -= 60;
        leaveTime[HR]++;
    }

    leaveTime[SEC] = 0; 

    printf("LEAVE AT Thursday ***" ANSI_COLOR_LIGHTRED" %02d:%02d:%02d "ANSI_COLOR_RESET"***\n", leaveTime[HR], leaveTime[MIN], leaveTime[MIN]);
}
void backOnSchedule(int* rem, int* nowTime)
{
    // projecting 8HRs per remaining days ex/today
    // Time to leave is 15:30 + remaining - 8:30*remDays
    
    int remDays = 4 - nowTime[DAY]; //remaining days in the week ex/today
    int leaveTime[3] = {0};

    leaveTime[HR] = rem[HR] - (8*remDays);
    leaveTime[MIN] = rem[MIN] - (30*remDays);
    while(leaveTime[MIN]<(-59))
    {
        leaveTime[MIN] += 60;
        leaveTime[HR]--;
    }

    leaveTime[HR] += 15;
    leaveTime[MIN] += 30;
    while(leaveTime[MIN] < 0)
    {
        leaveTime[MIN] += 60;
        leaveTime[HR]--;
    }
    while(leaveTime[MIN] > 59)
    {
        leaveTime[MIN] -= 60;
        leaveTime[HR]++;
    }

    leaveTime[SEC] = 0; 

    printf(ANSI_COLOR_YELLOWBLUE"Preferred"ANSI_COLOR_RESET" leave today at ***" ANSI_COLOR_WHITEBLUE" %02d:%02d:%02d "ANSI_COLOR_RESET"***\n", leaveTime[HR], leaveTime[MIN], leaveTime[MIN]);
    printf("else\n");
    offThursday(rem, nowTime);

}


//return remaining hours after now.
