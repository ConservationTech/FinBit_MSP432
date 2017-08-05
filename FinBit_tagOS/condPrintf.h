/*
 * condPrintf.h
 *
 *  Created on: Aug 2, 2017
 *      Author: dave
 */

#ifndef CONDPRINTF_H_
#define CONDPRINTF_H_

/* Comment out these define to disable functionality of verbose debug printf behavior
 * Think of these as boolean values, i.e.: 0 = false, 1 = true
 */

#define DEBUGPRINT  1
#define POSTPRINT   1

extern bool debugPrint;
extern bool postPrint;

#endif /* CONDPRINTF_H_ */
