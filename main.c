#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "biguint.h"

typedef struct tm date_time;

BigUint secondsInDay;
const uint32_t cumulativeMonthLength[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

#pragma #region utils
void printBigUint(const char *txt, BigUint r){
	char *buffer = biguintToStr(&r);
	printf("%s%s\n", txt, buffer);
	free(buffer);
}

char *readInput(){
	size_t size = 64; //Initial buffer size
	char *buffer = malloc(size);
	if(!buffer){
		return NULL; //malloc fail
	}
	
	size_t len = 0;
	int ch;
	
	while((ch = getchar()) != '\n' && ch != EOF) {
		if(len + 1 >= size){ //Resize buffer if needed
			size += 64;
			char *new = realloc(buffer, size);
			if(!new){
				free(buffer);
				return NULL; //realloc fail
			}
			buffer = new;
		}
		buffer[len++] = ch;
	}
	
	buffer[len] = '\0'; //Null endining
	
	return buffer;
}
#pragma  #endregion utils

#pragma #region dates
bool isYearLeap(uint32_t year){
	return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

uint32_t getFullDaysOfTheYear(uint32_t year, uint32_t month, uint32_t day){
	return cumulativeMonthLength[month] + (month > 1 ? isYearLeap(year) : 0) + day - 1;
}

uint32_t hmsToSeconds(uint32_t hours, uint32_t minutes, uint32_t seconds){
	return seconds + 60 * minutes + 3600 * hours;
}

uint32_t daysSinceYear1(uint32_t year){
	uint32_t centuries = year/100;
	uint32_t numberOfLeapCenturies = centuries/4;
	
	uint32_t daysInCenturies = centuries * 36524 + numberOfLeapCenturies;
	
	uint32_t yearsSince00 = year % 100;
	uint32_t numberOfLeapYearsSince00 = yearsSince00/4;
	
	return daysInCenturies + yearsSince00 * 365 + numberOfLeapYearsSince00;
}

uint32_t fullDaysSinceYear1(uint32_t year, uint32_t dayOfTheYear){
	return daysSinceYear1(year - 1) + dayOfTheYear;
}

uint32_t fullDaysSinceYear1_struct(date_time *t){
	return fullDaysSinceYear1(t->tm_year + 1900, t->tm_yday);
}

uint32_t fullDaysSinceYear1_data(uint32_t year, uint32_t month, uint32_t day){
	return fullDaysSinceYear1(year, getFullDaysOfTheYear(year, month, day));
}

BigUint dateTimeToSeconds(uint32_t fullDays, uint32_t hour, uint32_t minutes, uint32_t seconds){
	BigUint days = biguintNew(fullDays);
	BigUint daySeconds = biguintMultiply(&secondsInDay, &days);
	
	BigUint secondsToday = biguintNew(hmsToSeconds(hour, minutes, seconds));
	
	BigUint r = biguintSum(&secondsToday, &daySeconds);
	
	biguintFree(&days);
	biguintFree(&daySeconds);
	biguintFree(&secondsToday);
	
	return r;
}

BigUint dateTimeToSeconds_struct(uint32_t fullDays, date_time *t){
	return dateTimeToSeconds(fullDays, t->tm_hour, t->tm_min, t->tm_sec);
}

date_time *getNow(){
	time_t t = time(NULL);
	return localtime(&t);
}

char *getDayOfTheWeek(uint32_t index){
	switch(index){
		case 0: return "Monday";
		case 1: return "Tuesday";
		case 2: return "Wednesday";
		case 3: return "Thursday";
		case 4: return "Friday";
		case 5: return "Saturday";
		case 6: return "Sunday";
		default: return "";
	}
}
#pragma #endregion dates

bool parseInput(char *input, uint32_t *days, BigUint *totalSeconds){
	if(strlen(input) == 1 && (input[0] == 't' || input[0] == 'T')){
		date_time *strct = getNow();
		
		*days = fullDaysSinceYear1_struct(strct);
		
		*totalSeconds = dateTimeToSeconds_struct(*days, strct);
		
		return true;
	}else{
		int year, month, day, hour, minute, second;
		
		if(sscanf(input, "%d/%d/%d/%d/%d/%d", &year, &month, &day, &hour, &minute, &second) != 6){
			if(sscanf(input, "%d/%d/%d", &year, &month, &day) != 3){
				printf("Invalid input!\n");
				return false;
			}else{
				hour = 0;
				minute = 0;
				second = 0;
			}
		}
		
		if(month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59){
			printf("Invalid date/time!\n");
			return false;
		}
		
		*days = fullDaysSinceYear1_data(year, month - 1, day);
		
		*totalSeconds = dateTimeToSeconds(*days, hour, minute, second);
		
		return true;
	}
}

int main(){
	//Init
	secondsInDay = biguintNew(3600 * 24);
	
	printf("Welcome to this proleptic Gregorian calendar date calculator!\n\n");
	
	while(true){		
		//FIRST
		printf("Enter first date and time (YYYY/MM/DD/HH/MM/SS or YYYY/MM/DD or 'x' to exit): ");
		
		char *input = readInput();
		if(!input){
			printf("An error occured, try again\n\n");
			continue;
		}
		
		//Exiting
		if(strlen(input) == 1 && (input[0] == 'x' || input[0] == 'X')){
			free(input);
			break;
		}
		
		uint32_t days1;
		BigUint totalSeconds1;
		
		if(!parseInput(input, &days1, &totalSeconds1)){
			free(input);
			continue;
		}
		
		free(input);
		
		printf("Full days since year 1: %u\n", days1);
		printf("Day of the week: %s\n", getDayOfTheWeek(days1 % 7));
		printBigUint("Seconds since year 1: ", totalSeconds1);
		
		
		//SECOND
		printf("\nEnter second date and time (YYYY/MM/DD/HH/MM/SS or YYYY/MM/DD): ");
		
		input = readInput();
		if(!input){
			printf("An error occured, try again\n\n");
			biguintFree(&totalSeconds1);
			continue;
		}
		
		uint32_t days2;
		BigUint totalSeconds2;
		
		if(!parseInput(input, &days2, &totalSeconds2)){
			free(input);
			biguintFree(&totalSeconds1);
			continue;
		}
		
		free(input);
		
		printf("Full days since year 1: %u\n", days2);
		printf("Day of the week: %s\n", getDayOfTheWeek(days2 % 7));
		printBigUint("Seconds since year 1: ", totalSeconds2);
		
		//MAIN
		printf("\n");
		
		uint32_t daysDiff;
		if(days1 > days2){
			daysDiff = days1 - days2;
		}else{
			daysDiff = days2 - days1;
		}
		printf("Difference in days: %u\n", daysDiff);
		printf("Difference in years (estimate): %f\n", daysDiff / 365.0f);
		
		BigUint secDiff;
		
		if(biguintGreater(&totalSeconds1, &totalSeconds2)){
			secDiff = biguintSubtract(&totalSeconds1, &totalSeconds2);
		}else{
			secDiff = biguintSubtract(&totalSeconds2, &totalSeconds1);
		}
		
		printBigUint("Difference in seconds: ", secDiff);
		
		printf("\n\n");
		
		//Cleanup
		biguintFree(&totalSeconds1);
		biguintFree(&totalSeconds2);
		biguintFree(&secDiff);
	}
	
	//Cleanup
	biguintFree(&secondsInDay);
	
	return 0;
}