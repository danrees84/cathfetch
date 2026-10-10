/*
 * Gregorian Easter computus
 *
 * Calculates the date of Easter Sunday in the Gregorian calendar.
 *
 * Algorithm: J.-M. Oudin (1940)
 * Reference: US Naval Observatory
 * https://aa.usno.navy.mil/faq/easter
 *
 * All calculations use integer arithmetic, discarding division remainders.
 * Variable names and formulae follow the published algorithm.
 *
 * Input:  y = Gregorian year
 * Output: m = month (3 = March, 4 = April)
 *         d = day of month
 */

#include <chrono>
#include "easter.h"

std::chrono::year_month_day calculateEaster(int year)
{

	int y = year;

	// Century and 19-year lunar cycle.
	int c = y / 100;             
	int n = y - 19 * (y / 19); 
	int k = (c - 17) / 25; 
	int i = c - c / 4 - (c - k) / 3 + 19 * n + 15;

	// Calculate the ecclesiastical Paschal full moon offset.
	// Reduce to a 30-day cycle and apply the exceptional-case correction.
	i = i - 30 * ( i / 30 );
	i = i - ( i / 28 ) * ( 1 - ( i / 28 ) * ( 29 / ( i + 1 ) ) * ( ( 21 - n ) / 11 ) );


	// Apply the weekday correction to determine Easter Sunday.
	int j = y + y / 4 + i + 2 - c + c / 4;
	j = j - 7 * ( j / 7 );
	int l = i - j;

	// Convert the result into Gregorian month and day.
	int m = 3 + ( l + 40 ) / 44;
	int d = l + 28 - 31 * ( m / 4 );

	std::chrono::year_month_day easterDate{
		std::chrono::year{y},
			std::chrono::month{unsigned(m)},
			std::chrono::day{unsigned(d)}
	};

	return easterDate;

}
