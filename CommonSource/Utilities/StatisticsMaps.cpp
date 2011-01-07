#include "StatisticsMaps.h"

CStatisticsMaps::CStatisticsMaps( void )
	: nFragment(10000)
	, nFrangmentOver(0)
	, nFrangmentUnder(0)
	, minFragment(-99)
	, nReadLength(1000)
	, nReadLengthOver(0)
	, nReadLengthUnder(0)
	, nMultiplicity(1000)
	, nMultiplicityOver(0)
	, nMultiplicityUnder(0)
	, nMappingQuality(100)
	, nMappingQualityOver(0)
	, nMappingQualityUnder(0)
	, nMismatch(100)
	, nMismatchOver(0)
	, nMismatchUnder(0)
	, non_unique(0)
	, non_multiple(0)
	, unique_unique(0)
	, unique_multiple(0)
	, multiple_multiple(0)
{
	fragments        = new uint64_t [ nFragment ];
	readLengths      = new uint64_t [ nReadLength ];
	multiplicities   = new uint64_t [ nMultiplicity ];
	mappingQualities = new uint64_t [ nMappingQuality ];
	mismatches       = new uint64_t [ nMismatch ];
	
	memset ( fragments, 0, nFragment );
	memset ( readLengths, 0, nReadLength );
	memset ( multiplicities, 0, nMultiplicity );
	memset ( mappingQualities, 0, nMappingQuality );
	memset ( mismatches, 0, nMismatch );
}

CStatisticsMaps::~CStatisticsMaps( void ) {
	if ( fragments )        delete [] fragments;
	if ( readLengths )      delete [] readLengths;
	if ( multiplicities )   delete [] multiplicities;
	if ( mappingQualities ) delete [] mappingQualities;
	if ( mismatches )       delete [] mismatches;
}

void CStatisticsMaps::Clear( void ) {
	
	nFrangmentOver       = 0;
	nFrangmentUnder      = 0;
	nReadLengthOver      = 0;
	nReadLengthUnder     = 0;
	nMultiplicityOver    = 0;
	nMultiplicityUnder   = 0;
	nMappingQualityOver  = 0;
	nMappingQualityUnder = 0;
	nMismatchOver        = 0;
	nMismatchUnder       = 0;
	non_unique           = 0;
	non_multiple         = 0;
	unique_unique        = 0;
	unique_multiple      = 0;
	multiple_multiple    = 0;

	memset ( fragments, 0, nFragment );
	memset ( readLengths, 0, nReadLength );
	memset ( multiplicities, 0, nMultiplicity );
	memset ( mappingQualities, 0, nMappingQuality );
	memset ( mismatches, 0, nMismatch );

}

inline void CStatisticsMaps::SaveFragment( const vector<Alignment>& als1, const vector<Alignment>& als2, const SequencingTechnologies& tech ) {
	// true: +; false: -.
	bool strand1 = !als1[0].IsReverseStrand;
	bool strand2 = !als2[0].IsReverseStrand;
	bool okay    = false;
	int64_t length = 0;
	switch( tech ) {
		case ST_454:
			if ( strand1 == strand2 ) {
				okay = true;
				length = strand1 ? als1[0].ReferenceEnd - als2[0].ReferenceBegin + 1 : als2[0].ReferenceEnd - als1[0].ReferenceBegin + 1;
			}
			break;
		case ST_SOLID:
			if ( strand1 == strand2 ) {
				okay = true;
				length = strand1 ? als2[0].ReferenceEnd - als1[0].ReferenceBegin + 1 : als1[0].ReferenceEnd - als2[0].ReferenceBegin + 1;
			}
			break;
		default:
			if ( strand1 != strand2 ) {
				okay = true;
				length = strand1 ? als2[0].ReferenceEnd - als1[0].ReferenceBegin + 1 : als1[0].ReferenceEnd - als2[0].ReferenceBegin + 1;
			}
	}

	if ( okay ) {
		if ( length < minFragment )
			nFrangmentUnder++;
		else if ( length > ( minFragment + (int64_t)nFragment ) )
			nFrangmentOver++;
		else
			fragments[ length - minFragment ]++;
	}
}

inline void CStatisticsMaps::SaveReadLength( const Mosaik::Mate& m ) {
	
	unsigned int length = m.Bases.Length();
	if ( length > nReadLength ) {
		nReadLengthOver++;
	} else {
		readLengths[ length ]++;
	}
}

inline void CStatisticsMaps::SaveMultiplicity( vector<Alignment>& als ) {
	
	unsigned int nAlignment = als.size();
	if ( nAlignment > nMultiplicity ) {
		nMappingQualityOver++;
	} else {
		multiplicities[ nAlignment ]++;
	}
}

inline void CStatisticsMaps::SaveMappingQuality( vector<Alignment>& als ) {
	
	for ( vector<Alignment>::iterator ite = als.begin(); ite !=als.end(); ++ite ) {
		unsigned char mq = ite->Quality;
		// mq shouldn't larger than nMappingQuality = 100
		if ( mq > nMappingQuality ) {
			nMappingQualityOver++;
		} else {
			mappingQualities[ mq ]++;
		}
	}
}

inline void CStatisticsMaps::SaveMismatch( vector<Alignment>& als ) {
	
	for ( vector<Alignment>::iterator ite = als.begin(); ite !=als.end(); ++ite ) {
		unsigned short mm = ite->NumMismatches;
		if ( mm > nMismatch ) {
			nMismatchOver++;
		} else {
			mismatches[ mm ]++;
		}
	}
}

inline void CStatisticsMaps::PrintMap( FILE *const fOut, const char* title, const uint64_t& size, const uint64_t& over, const uint64_t& under, const uint64_t *const array, const int64_t& start  ) {
	
	fprintf( fOut, "\n%s\n", title );

	// calculate sum and count
	int64_t sum   = 0;
	int64_t count = 0;
	for ( uint64_t i = 0; i < size; ++i ) {
		sum += array[i] * ( start + i );
		count += array[i];
	}

	double mean = sum / (double) count;
	double std = sqrt( (( pow(sum,2.0)) -(( 1.0/count) * (pow(sum,2.0))))/ (count -1.0));
	fprintf( fOut, "\tTOT\tMEAN\tSTD\tIN\tOVER\tUNDER\n" );
	fprintf( fOut, "\t%lu\t%10.3f\t%10.3f\t%lu\t%lu\t%lu\n", count + over + under, mean, std, count, over, under);
	fprintf( fOut, "\tbin\tx\tn\tcum\n");

	uint64_t cum = under;
	for ( uint64_t i = 0; i < size; ++i ) {
		if ( array[i] == 0 )
			continue;

		cum += array[i];
		fprintf( fOut, "\t%lu\t%lu\t%lu\t%lu\n", i, i + start, array[i], cum );
	}
}

void CStatisticsMaps::PrintMaps( const char* filename ) {
	FILE* fOut;
	fOut = fopen( filename, "w" );
	if ( fOut != NULL ) {
		PrintMap( fOut, "LF fragment mapping length",   nFragment,       nFrangmentOver,      nFrangmentUnder,      fragments,        minFragment );
		PrintMap( fOut, "LR read length",               nReadLength,     nReadLengthOver,     nReadLengthUnder,     readLengths,      0 );
		PrintMap( fOut, "NA read mapping multiplicity", nMultiplicity,   nMultiplicityOver,   nMultiplicityUnder,   multiplicities,   0 );
		PrintMap( fOut, "RQ read map quality",          nMappingQuality, nMappingQualityOver, nMappingQualityUnder, mappingQualities, 0 );
		PrintMap( fOut, "MM read map mismatch",         nMismatch,       nMismatchOver,       nMismatchUnder,       mismatches,       0 );

		// print pair combinations
		uint64_t total = non_unique + non_multiple + unique_unique + unique_multiple + multiple_multiple;
		fprintf( fOut, "\nRC pair multiplicity combinations\n");
		fprintf( fOut, "\tTOT\tMEAN\tSTD\tIN\tOVER\tUNDER\n" );
		fprintf( fOut, "\t%lu\t0\t0\t%lu\t0\t0\n", total, total );
		fprintf( fOut, "\tbin\tcombo\tn\tcum\tlabel\n");
		fprintf( fOut, "\t1\t1\t%lu\t%lu\t0-1\n", non_unique,        non_unique );
		fprintf( fOut, "\t2\t2\t%lu\t%lu\t0-N\n", non_multiple,      non_unique + non_multiple );
		fprintf( fOut, "\t4\t4\t%lu\t%lu\t1-1\n", unique_unique,     non_unique + non_multiple + non_multiple );
		fprintf( fOut, "\t5\t5\t%lu\t%lu\t1-N\n", unique_multiple,   non_unique + non_multiple + non_multiple + unique_multiple );
		fprintf( fOut, "\t8\t8\t%lu\t%lu\tN-N\n", multiple_multiple, total );
		fclose( fOut );
	} else {
		printf("ERROR: The statistics maps cannot be printed out.\n");
		exit(1);
	}
}

void CStatisticsMaps::SaveRecord( const Mosaik::Read& r, vector<Alignment>& als1, vector<Alignment>& als2, const bool isPairedEnd, const SequencingTechnologies& tech ) {
	
	//bool isUU = isPairedEnd && ( als1.size() == 1 ) && ( als2.size() == 1 );

	// calculate fragment length
	//if ( isUU ) {
	//	
	//}
	
	SaveReadLength( r.Mate1 );
	SaveMultiplicity( als1 );
	SaveMappingQuality( als1 );
	SaveMismatch( als1 );

	if ( isPairedEnd ) {
		SaveReadLength( r.Mate2 );
		SaveMultiplicity( als2 );
		SaveMappingQuality( als2 );
		SaveMismatch( als2 );

		if ( ( ( als1.size() == 0 ) && ( als2.size() == 1 ) ) || ( ( als1.size() == 1 ) && ( als2.size() == 0 ) ) )
			non_unique++;

		if ( ( ( als1.size() == 0 ) && ( als2.size() > 1 ) ) || ( ( als1.size() > 1 ) && ( als2.size() == 0 ) ) )
			non_multiple++;

		if ( ( als1.size() == 1 ) && ( als2.size() == 1 ) ) {
			unique_unique++;
			SaveFragment( als1, als2, tech );
		}

		if ( ( ( als1.size() == 1 ) && ( als2.size() > 1 ) ) || ( ( als1.size() > 1 ) && ( als2.size() == 1 ) ) )
			unique_multiple++;

		if ( ( als1.size() > 0 ) && ( als2.size() > 1 ) )
			multiple_multiple++;
	}
}