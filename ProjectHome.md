# News #

2014-03-26 - A bug causing incorrect bases of reverse complement alignments has been fixed. Please check any version greater than **2.2.19** for the fix.

2014-03-05 - MOSAIK is published on PLoS ONE, [http://dx.plos.org/10.1371/journal.pone.0090581](http://dx.plos.org/10.1371/journal.pone.0090581).

2013-09-20 - **MOSAIK 2.2** is about <font color='red'>four-fold faster</font> than previous by improving the Smith-Waterman (SW) strategy and using the SIMD SW (https://github.com/mengyao/Complete-Striped-Smith-Waterman-Library). This is also the version that we
use on 1000 Genomes Project (http://www.1000genomes.org/) Phase-III datasets.

2011-11-08 - **MOSAIK 2.1** was released. We are very happy to introduce this version and we have been working hard on this version for a long time to shorten the pipeline and fix bugs. In the current pipeline, MosaikAligner generates BAMs as outputs directly and thus MosaikSort and MosaikText in the old pipeline are unnecessary for generating BAMs and no longer maintained. We also change our version control system to Git which is easier and more convenient to use.

# [QuickStart](https://github.com/wanpinglee/MOSAIK/wiki/QuickStart) #
[https://github.com/wanpinglee/MOSAIK/wiki/QuickStart](https://github.com/wanpinglee/MOSAIK/wiki/QuickStart)

# Introduction #

**MOSAIK** is a reference-guided assembler comprising of two main modular programs:

  * MosaikBuild
  * MosaikAligner

MosaikBuild converts various sequence formats into Mosaik’s native read format. MosaikAligner pairwise aligns each read to a specified series of reference sequences and produces BAMs as outputs.

At this time, the workflow consists of supplying sequences in FASTA, FASTQ, Illumina Bustard & Gerald, or SRF file formats and producing results in the BAM format.

# What's new? #
  1. <p>A new neural-net for mapping quality (MQ) calibration is introduced. Initial testing using simulated reads shows that this method improve the accuracy compared to the previous MQ scheme.</p>
  1. <p>The overall <b>alignment speed is much quicker</b> now due to a banded Smith-Waterman algorithm implementation. Longer Roche 454 reads align much quicker than before.<p />
<ol><li><p>A <b>local alignment search option</b> has been added to help rescue mates in paired-end/mate-pair reads that may be missing due to highly repetitive regions in the genome.<p />
</li><li><p><b>SOLiD support</b> has finally come of age. MOSAIK imports and aligns SOLiD reads in colorspace, but now seamlessly converts the alignments back into basespace. No more downstream bioinformatics headaches.<p />
</li><li><p>Robust support for the BAM alignment file formats.<p />
</li><li><p>The command line parameters have been cleaned up and sensible default parameters have been chosen. This cuts down the ridiculously long command-lines to simply specifying an input file and an output file in most cases.<p /></li></ol>

<h1>What makes MOSAIK different?</h1>

Unlike many current read aligners, <b>MOSAIK</b> produces gapped alignments using the Smith-Waterman algorithm.<br>
<b>MOSAIK</b> is written in highly portable C++ and currently targetted for the following platforms: Microsoft Windows, Apple Mac OS X, FreeBSD, and Linux operating systems. Other platforms can easily be supported upon request.<br>
<br>
<b>MOSAIK</b> is multithreaded. If you have a machine with 8 processors, you can use all 8 processors to align reads faster while using the same memory footprint as when using one processor.<br>
<br>
<b>MOSAIK</b> supports multiple sequencing technologies. In addition to legacy technologies such as Sanger capillary sequencing, our program supports next generation technologies such as Roche 454, Illumina, AB SOLiD, and experimental support for the Helicos Heliscope.<br>
<br>
<h2>Finally Open Source!</h2>

By popular demand, MOSAIK is now dual-licensed under both the GPL 2.0+ and the MIT License.<br>
<br>
We are also very eager to add contributors to the MOSAIK code base. Let us know if you're interested.<br>
<br>
<h1>Citation</h1>

<a href='http://dx.plos.org/10.1371/journal.pone.0090581'>http://dx.plos.org/10.1371/journal.pone.0090581</a>

We ask that you cite this paper if you use MOSAIK in work that leads to<br>
publication.<br>
<br>
<br>
<h1>Help!</h1>

For MOSAIK related questions, please send Wan-Ping Email, <a href='mailto:wanping.lee@bc.edu'>wanping.lee@bc.edu</a>.