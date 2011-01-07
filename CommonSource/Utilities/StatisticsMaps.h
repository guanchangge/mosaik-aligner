#ifndef _STATISTICSMAPS_H_
#define _STATISTICSMAPS_H_

#include <stdio.h>
#include <stdint.h>

#include <cmath>
#include <vector>

#include "Alignment.h"
#include "Read.h"
#include "SequencingTechnologies.h"

using namespace std;

class CStatisticsMaps {
	public:
		CStatisticsMaps( void );
		~CStatisticsMaps( void );
		
		void SaveRecord( const Mosaik::Read& m, vector<Alignment>& als1, vector<Alignment>& als2, const bool isPairedEnd, const SequencingTechnologies& tech );
		void PrintMaps( const char* filename );
		void Clear( void );

	private:

		inline void SaveFragment( const vector<Alignment>& als1, const vector<Alignment>& als2, const SequencingTechnologies& tech );
		inline void SaveReadLength( const Mosaik::Mate& r );
		inline void SaveMultiplicity( vector<Alignment>& als );
		inline void SaveMappingQuality( vector<Alignment>& als );
		inline void SaveMismatch( vector<Alignment>& als );
		inline void PrintMap( FILE *const fOut, const char* title, const uint64_t& size, const uint64_t& over, const uint64_t& under, const uint64_t *const array, const int64_t& start  );


		// fragment length map
		const uint64_t nFragment;
		uint64_t nFrangmentOver;
		uint64_t nFrangmentUnder;
		short    minFragment;
		uint64_t* fragments;
		// read length map
		const uint64_t nReadLength;
		uint64_t nReadLengthOver;
		uint64_t nReadLengthUnder;
		uint64_t* readLengths;
		// multiplicity map
		const uint64_t nMultiplicity;
		uint64_t nMultiplicityOver;
		uint64_t nMultiplicityUnder;
		uint64_t* multiplicities;
		// mapping quality map
		const uint64_t nMappingQuality;
		uint64_t nMappingQualityOver;
		uint64_t nMappingQualityUnder;
		uint64_t* mappingQualities;
		// mismatch map
		const uint64_t nMismatch;
		uint64_t nMismatchOver;
		uint64_t nMismatchUnder;
		uint64_t* mismatches;
		// pair map
		uint64_t non_unique;
		uint64_t non_multiple;
		uint64_t unique_unique;
		uint64_t unique_multiple;
		uint64_t multiple_multiple;

};

#endif