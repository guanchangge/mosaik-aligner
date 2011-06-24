// ***************************************************************************
// CAlignmentThread - aligns all of the reads within a worker thread.
// ---------------------------------------------------------------------------
// (c) 2006 - 2009 Michael Str�mberg & Wan-Ping Lee
// Marth Lab, Department of Biology, Boston College
// ---------------------------------------------------------------------------
// Dual licenced under the GNU General Public License 2.0+ license or as
// a commercial license with the Marth Lab.
// ***************************************************************************

#pragma once

#include <limits.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "AbstractDnaHash.h"
#include "AlignmentWriter.h"
#include "BandedSmithWaterman.h"
#include "BamWriter.h"
#include "BestNSecondBestSelection.h"
#include "CigarTager.h"
#include "ColorspaceUtilities.h"
#include "MdTager.h"
#include "NaiveAlignmentSet.h"
#include "PairwiseUtilities.h"
#include "PosixThreads.h"
#include "ReadReader.h"
#include "ReferenceSequence.h"
#include "SequenceUtilities.h"
#include "SmithWatermanGotoh.h"
#include "StatisticsMaps.h"
#include "ZaTager.h"

using namespace std;

#define ALLOCATION_EXTENSION 10

// add our alignment status codes
typedef unsigned char AlignmentStatusType;
const AlignmentStatusType ALIGNMENTSTATUS_GOOD        = 10;
const AlignmentStatusType ALIGNMENTSTATUS_TOOSHORT    = 20;
const AlignmentStatusType ALIGNMENTSTATUS_FAILEDHASH  = 30;
const AlignmentStatusType ALIGNMENTSTATUS_FILTEREDOUT = 40;

class CAlignmentThread {
public:
	// our enumerated alignment algorithms
	enum AlignerAlgorithmType {
		AlignerAlgorithm_FAST,
		AlignerAlgorithm_SINGLE,
		AlignerAlgorithm_MULTI,
		AlignerAlgorithm_ALL
	};
	// our enumerated alignment modes
	enum AlignerModeType {
		AlignerMode_UNIQUE,
		AlignerMode_ALL
	};
	// define our alignment configuration structure
	struct AlignerSettings {
		string AlignedReadReportFilename;
		string BasespaceReferenceFilename;
		string InputReadArchiveFilename;
		string JumpFilenameStub;
		string OutputReadArchiveFilename;
		string ReferenceFilename;
		//string UnalignedReadReportFilename;
		unsigned int AllocatedReadLength;
		unsigned int Bandwidth;
		unsigned int LocalAlignmentSearchRadius;
		unsigned int MedianFragmentLength;
		unsigned int NumCachedHashes;
		unsigned short AlignmentCandidateThreshold;
		unsigned short HashPositionThreshold;
		unsigned short HashRegionThreshold;
		unsigned char HashSize;
		unsigned char NumThreads;
		SequencingTechnologies SequencingTechnology;
	};
	// stores the filter settings
	struct FilterSettings {
		bool UseMinAlignmentFilter;
		bool UseMinAlignmentPercentFilter;
		bool UseMismatchFilter;
		bool UseMismatchPercentFilter;

		double MinPercentAlignment;
		double MaxMismatchPercent;
		unsigned int MinAlignment;
		unsigned int MaxNumMismatches;

		FilterSettings()
			: UseMinAlignmentFilter(CPairwiseUtilities::UseMinAlignmentFilter)
			, UseMinAlignmentPercentFilter(CPairwiseUtilities::UseMinAlignmentPercentFilter)
			, UseMismatchFilter(CPairwiseUtilities::UseMismatchFilter)
			, UseMismatchPercentFilter(CPairwiseUtilities::UseMismatchPercentFilter)
			, MinPercentAlignment(CPairwiseUtilities::MinPercentAlignment)
			, MaxMismatchPercent(CPairwiseUtilities::MaxMismatchPercent)
			, MinAlignment(CPairwiseUtilities::MinAlignment)
			, MaxNumMismatches(CPairwiseUtilities::MaxNumMismatches)
		{}
	};
	// define our boolean flags structure
	struct FlagData {
		bool EnableColorspace;
		bool IsAligningAllReads;
		bool IsQuietMode;
		//bool IsReportingUnalignedReads;
		bool IsUsingAlignmentCandidateThreshold;
		bool IsUsingHashPositionThreshold;
		bool IsUsingHashRegionThreshold;
		bool IsUsingJumpDB;
		bool KeepJumpKeysInMemory;
		bool KeepJumpPositionsInMemory;
		bool OutputMultiply;
		bool UseAlignedReadLengthForMismatchCalculation;
		bool UseBandedSmithWaterman;
		bool UseLocalAlignmentSearch;
		bool UsePairedEndOutput;
		bool UseLowMemory;
		bool UseBamOutput;
		bool UseArchiveOutput;
		bool SaveMultiplyBam;
		bool SaveUnmappedBasesInArchive;

		FlagData()
			: EnableColorspace(false)
			, IsAligningAllReads(false)
			, IsQuietMode(false)
			//, IsReportingUnalignedReads(false)
			, IsUsingAlignmentCandidateThreshold(false)
			, IsUsingHashPositionThreshold(false)
			, IsUsingHashRegionThreshold(false)
			, IsUsingJumpDB(false)
			, KeepJumpKeysInMemory(false)
			, KeepJumpPositionsInMemory(false)
			, OutputMultiply(false)
			, UseAlignedReadLengthForMismatchCalculation(false)
			, UseBandedSmithWaterman(false)
			, UseLocalAlignmentSearch(false)
			, UsePairedEndOutput(false)
			, UseLowMemory(false)
			, UseBamOutput(false)
			, UseArchiveOutput(false)
			, SaveMultiplyBam(false)
			, SaveUnmappedBasesInArchive(false)
		{}
	};
	// stores the statistical counters
	struct StatisticsCounters {
		uint64_t AdditionalLocalMates;
		uint64_t AlignedReads;
		uint64_t AlignmentCandidates;
		uint64_t BothNonUniqueReads;
		uint64_t BothUniqueReads;
		uint64_t FailedHashMates;
		uint64_t FilteredOutMates;
		uint64_t MateBasesAligned;
		uint64_t NonUniqueMates;
		uint64_t OneNonUniqueReads;
		uint64_t OrphanedReads;
		uint64_t ShortMates;
		uint64_t UnalignedReads;
		uint64_t UniqueMates;
		unsigned char StatMappingQuality;

		StatisticsCounters() 
			: AdditionalLocalMates(0)
			, AlignedReads(0)
			, AlignmentCandidates(0)
			, BothNonUniqueReads(0)
			, BothUniqueReads(0)
			, FailedHashMates(0)
			, FilteredOutMates(0)
			, MateBasesAligned(0)
			, NonUniqueMates(0)
			, OneNonUniqueReads(0)
			, OrphanedReads(0)
			, ShortMates(0)
			, UnalignedReads(0)
			, UniqueMates(0)
			, StatMappingQuality(20)
		{}
	};
	// bam writers
	struct BamWriters {
		BamHeader mHeader; // multiply alignments
		BamHeader sHeader; // special alignments
		BamHeader uHeader; // unaligned reads
		BamHeader rHeader; // regular bam

		CBamWriter mBam; // multiply alignments
		CBamWriter sBam; // special alignments
		CBamWriter uBam; // unaligned reads
		CBamWriter rBam;
	};
	// special reference 
	struct SReference {
		bool     enable;
		bool     found;
		uint64_t begin;
		uint64_t nReference;
		double   count;
		string   prefix;

		SReference()
			: enable(false)
			, found(false)
			, begin(0)
			, nReference(0)
			, count(0)
		{}
	};
	// constructor
	CAlignmentThread(
		const AlignerAlgorithmType& algorithmType, 
		const FilterSettings&       filters, 
		const FlagData&             flags, 
		const AlignerModeType&      algorithmMode, 
		char*                       pReference, 
		const unsigned int          referenceLen, 
		CAbstractDnaHash*           pDnaHash, 
		const AlignerSettings&      settings, 
		unsigned int*               pRefBegin, 
		unsigned int*               pRefEnd, 
		char**                      pRefSpecies, 
		bool*                       pRefSpecial,
		char**                      pBsRefSeqs, 
		const SReference&           SpecialReference,
		map <unsigned int, MosaikReadFormat::ReadGroup>* pReadGroupsMap,
		const unsigned int          referenceOffset
	);

	// destructor
	~CAlignmentThread(void);
	// define our thread data structure
	struct ThreadData {
		AlignerAlgorithmType Algorithm;
		AlignerModeType      Mode;
		AlignerSettings      Settings;
		FilterSettings       Filters;
		FlagData             Flags;
		StatisticsCounters*  pCounters;
		CStatisticsMaps*     pMaps;
		CAbstractDnaHash*    pDnaHash;
		MosaikReadFormat::CReadReader*      pIn;
		MosaikReadFormat::CAlignmentWriter* pOut;
		//FILE*         pUnalignedStream;
		unsigned int  ReferenceLen;
		char*         pReference;
		unsigned int* pRefBegin;
		unsigned int* pRefEnd;
		char**        pRefSpecies;
		bool*         pRefSpecial;
		uint64_t*     pReadCounter;
		bool          IsPairedEnd;
		char**        pBsRefSeqs;
		BamWriters*   pBams;
		SReference    SpecialReference;
		map< unsigned int, MosaikReadFormat::ReadGroup >* pReadGroups;
		unsigned int  ReferenceOffset;
	};
	// aligns the read archive
	void AlignReadArchive(
		MosaikReadFormat::CReadReader*      pIn, 
		MosaikReadFormat::CAlignmentWriter* pOut, 
		//FILE*            pUnalignedStream,
		uint64_t*        pReadCounter,
		bool             isPairedEnd, 
		CStatisticsMaps* pMaps, 
		BamWriters*      pBams,
		unsigned char    statMappingQuality
	);
	// activates the current alignment thread
	static void* StartThread(void* arg);
	// register our thread mutexes
	static pthread_mutex_t mGetReadMutex;
	//static pthread_mutex_t mReportUnalignedMate1Mutex;
	//static pthread_mutex_t mReportUnalignedMate2Mutex;
	static pthread_mutex_t mSaveReadMutex;
	static pthread_mutex_t mStatisticsMutex;
	static pthread_mutex_t mStatisticsMapsMutex;
	static pthread_mutex_t mSaveMultipleBamMutex;
	static pthread_mutex_t mSaveUnmappedBamMutex;
	static pthread_mutex_t mSaveSpecialBamMutex;
	// stores the statistical counters
	StatisticsCounters mStatisticsCounters;
private:
	// define a comparison function for sorting our hash regions
	struct SortHashRegionByLength {
		bool operator()(const HashRegion& hr1, const HashRegion& hr2) {
			return (hr2.End - hr2.Begin) < (hr1.End - hr1.Begin);
		}
	};
	// our local alignment model data structure used in mate rescue
	struct LocalAlignmentModel {
		bool IsTargetBeforeUniqueMate;
		bool IsTargetReverseStrand;

		LocalAlignmentModel(void)
			: IsTargetBeforeUniqueMate(false)
			, IsTargetReverseStrand(false)
		{}
	};
	// aligns the read against the reference sequence and returns true if the read was aligned
	bool AlignRead(CNaiveAlignmentSet& alignments, const char* query, const char* qualities, const unsigned int queryLength, AlignmentStatusType& status);
	// aligns the read against a specified hash region using Smith-Waterman-Gotoh
	void AlignRegion(const HashRegion& r, Alignment& alignment, char* query, unsigned int queryLength, unsigned int extensionBases);
	// returns true if the alignment passes all of the user-specified filters
	bool ApplyReadFilters(Alignment& al, const char* bases, const char* qualities, const unsigned int queryLength);
	// creates the hash for a supplied fragment
	void CreateHash(const char* fragment, const unsigned char fragmentLen, uint64_t& key);
	// consolidates hash hits into a read candidate (fast algorithm)
	void GetFastReadCandidate(HashRegion& region, char* query, const unsigned int queryLength, MhpOccupancyList* pMhpOccupancyList);
	// consolidates hash hits into read candidates
	void GetReadCandidates(vector<HashRegion>& regions, char* query, const unsigned int queryLength, MhpOccupancyList* pMhpOccupancyList);
	// settles the local Smith-Waterman window
	bool SettleLocalSearchRegion( const LocalAlignmentModel& lam, const unsigned int refIndex, 
		const unsigned int uniqueBegin, const unsigned int uniqueEnd, unsigned int& localSearchBegin, unsigned int& localSearchEnd );
	// attempts to rescue the mate paired with a unique mate
	bool RescueMate(const LocalAlignmentModel& lam, const CMosaikString& bases, const unsigned int begin, 
		const unsigned int end, const unsigned int refIndex, Alignment& al);
	// Prepare bam required info
	void SetRequiredInfo ( Alignment& al, const Alignment& mate, const Mosaik::Mate& m, const Mosaik::Read& r, const bool& isPair,
		const bool& isProperPair, const bool& isFirstMate, const bool& isPairTech, const bool& isItselfMapped, const bool& isMateMapped);
	// denotes the active alignment algorithm
	AlignerAlgorithmType mAlgorithm;
	// denotes the active alignment mode
	AlignerModeType mMode;
	// stores the alignment configuration
	AlignerSettings mSettings;
	// stores the filter configuration
	FilterSettings mFilters;
	// stores our boolean flags
	FlagData mFlags;
	// the reference sequence
	char* mReference;
	// the sepcial references
	SReference mSReference;
	// our forward and reverse complement copy of the read
	char* mForwardRead;
	char* mReverseRead;
	// the length of the reference sequence
	unsigned int mReferenceLength;
	// the hash-table associated with the specified alignment algorithm
	CAbstractDnaHash* mpDNAHash;
	// our Smith-Waterman-Gotoh local alignment algorithms
	CSmithWatermanGotoh mSW;
	CBandedSmithWaterman mBSW;
	// our base quality LUT
	double mBaseQualityLUT[100];
	// our reference sequence LUTs
	unsigned int* mReferenceBegin;
	unsigned int* mReferenceEnd;
	char** mReferenceSpecies;
	bool*  mReferenceSpecial;
	// our alignment quality constants
	static const double P_ERR_REF;
	static const double P_CORR_REF;
	static const double ONE_THIRD;
	static const double TWO_NINTHS;
	// for soft clips
	//const unsigned int softClippedIdentifierLength;
	char* softClippedIdentifier;
	// our colorspace to basespace converter
	CColorspaceUtilities mCS;
	vector<ReferenceSequence> mpBsRefSeqs;
	// read groups map
	map<unsigned int, MosaikReadFormat::ReadGroup>* mReadGroupsMap;
	// reference offset used for low-memory multiply-mapped bam
	unsigned int mReferenceOffset;
	// ZA tagers
	CZaTager za1, za2;
	// MQ recalculation
	//static const double UU_COEFFICIENT;
	//static const double UU_INTERCEPT;
	//static const double UM_COEFFICIENT;
	//static const double UM_INTERCEPT;
	//static const double MM_COEFFICIENT;
	//static const double MM_INTERCEPT;
	// best and second best utilities
	void SelectBestNSecondBest ( vector<Alignment>& mate1Set, vector<Alignment>& mate2Set, const bool isMate1Aligned, const bool isMate2Aligned);
	void ProcessSpecialAlignment ( vector<Alignment>& mate1Set, vector<Alignment>& mate2Set, 
		Alignment& mate1SpecialAl, Alignment& mate2SpecialAl, bool& mate1Special, bool& mate2Special );
	// treat the best alignment as an unique mapping and than turn on local search
	bool TreatBestAsUnique( vector<Alignment>& mateSet );
	
	static inline bool LessThanMQ ( const Alignment& al1, const Alignment& al2) {
		return al1.Quality < al2.Quality;
	}
	
};
