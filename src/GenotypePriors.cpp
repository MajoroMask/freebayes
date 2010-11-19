#include "GenotypePriors.h"


long double alleleFrequencyProbability(map<int, int> alleleFrequencyCounts, long double theta) {

    int M = 0;
    long double p = 1;

    for (map<int, int>::iterator f = alleleFrequencyCounts.begin(); f != alleleFrequencyCounts.end(); ++f) {
        int frequency = f->first;
        int count = f->second;
        M += frequency * count;
        p *= pow(theta, count) / pow(frequency, count) * factorial(count);
    }

    long double thetaH = 1;
    for (int h = 1; h < M; ++h)
        thetaH *= theta + h;

    return factorial(M) / (theta * thetaH) * p;

}

// Implements Ewens' Sampling Formula, which provides probability of a given
// partition of alleles in a sample from a population
long double alleleFrequencyProbabilityln(map<int, int> alleleFrequencyCounts, long double theta) {

    int M = 0; // multiplicity of site
    long double p = 0;
    long double thetaln = log(theta);

    for (map<int, int>::iterator f = alleleFrequencyCounts.begin(); f != alleleFrequencyCounts.end(); ++f) {
        int frequency = f->first;
        int count = f->second;
        M += frequency * count;
        p += powln(thetaln, count) - powln(log(frequency), count) + factorialln(count);
    }

    long double thetaH = 0;
    for (int h = 1; h < M; ++h)
        thetaH += log(theta + h);

    return factorialln(M) - (thetaln + thetaH) + p;

}

// TODO XXX only works for the diploid case!!!
long double probabilityDiploidGenotypeComboGivenAlleleFrequencyln(GenotypeCombo& genotypeCombo, Allele& allele) {

    int n = genotypeCombo.size();
    int h = 0; // number of heterozygotes
    int f = genotypeCombo.alleleFrequency(allele);

    for (GenotypeCombo::iterator gc = genotypeCombo.begin(); gc != genotypeCombo.end(); ++gc) {
        if (!gc->genotype->homozygous)
            ++h;
    }

    return powln(log(2), h) - (factorialln(2 * n) - (factorialln(f) + factorialln(2 * n - f)));

}


// TODO this should return the probability of a given combination of genotypes
// given an allele frequency following the generalized HWE.
long double probabilityGenotypeComboGivenAlleleFrequencyln(GenotypeCombo& genotypeCombo, Allele& allele) {

    int n = genotypeCombo.numberOfAlleles();
    int h = 0; // number of heterozygotes
    int f = genotypeCombo.alleleFrequency(allele);
    long double lnploidyscalar = 0;

    for (GenotypeCombo::iterator gc = genotypeCombo.begin(); gc != genotypeCombo.end(); ++gc) {
        if (!gc->genotype->homozygous) {
            lnploidyscalar += log(gc->genotype->ploidy);
            ++h;
        }
    }

    return lnploidyscalar - (factorialln(n) - (factorialln(f) + factorialln(n - f)));

}


// core calculation of genotype combination likelihoods
//
void
genotypeCombinationPriorProbability(
        vector<GenotypeComboResult>& genotypeComboProbs,
        vector<GenotypeCombo>& bandedCombos,
        Allele& refAllele,
        long double theta,
        bool pooled) {

    for (vector<GenotypeCombo>::iterator combo = bandedCombos.begin(); combo != bandedCombos.end(); ++combo) {

        // when we are operating on pooled samples, we will not be able to
        // ascertain the number of heterozygotes in the pool,
        // rendering P(Genotype combo | Allele frequency) meaningless
        long double priorProbabilityOfGenotypeComboG_Af = 0;
        if (!pooled) {
            priorProbabilityOfGenotypeComboG_Af = probabilityGenotypeComboGivenAlleleFrequencyln(*combo, refAllele);
        }
        // Ewens' Sampling Formula
        long double priorProbabilityOfGenotypeComboAf = 
            alleleFrequencyProbabilityln(combo->countFrequencies(), theta);
        long double priorProbabilityOfGenotypeCombo = 
            priorProbabilityOfGenotypeComboG_Af + priorProbabilityOfGenotypeComboAf;
        long double priorComboProb = priorProbabilityOfGenotypeCombo + combo->prob;

        genotypeComboProbs.push_back(GenotypeComboResult(&*combo,
                    priorComboProb,
                    combo->prob,
                    priorProbabilityOfGenotypeCombo,
                    priorProbabilityOfGenotypeComboG_Af,
                    priorProbabilityOfGenotypeComboAf));

    }
}