#include <stdio.h>

#include "fasta.h"
#include "fastq.h"
#include "sam.h"
#include <cstr.h>

// We hardwire that the alphabet is acgt + $
// and have a global variable for the alphabet.
// It is initialised in main().
static cstr_alphabet ALPHA;

// Linked list of preprocessed arrays
struct sa
{
    const char *chr_name;  // chromosome name; data stored in fasta record
    cstr_const_sslice x;   // the chromosome seq. Stored in the fasta record
    cstr_suffix_array *sa; // the suffix array. We own it.
    struct sa *next;       // next suffix array in the linked list.
};
static struct sa *process_fasta_rec(struct fasta_record *fa_rec,
                                    struct sa *next)
{
    struct sa *sa = cstr_malloc(sizeof *sa);
    sa->chr_name = fa_rec->name;
    sa->x = fa_rec->seq;
    sa->sa = cstr_alloc_uislice(sa->x.len);

    cstr_uislice *u_buf = cstr_alloc_uislice(sa->x.len);
    bool map_ok = cstr_alphabet_map_to_uint(*u_buf, fa_rec->seq, &ALPHA);
    if (!map_ok)
    {
        abort(); // A little drastic, but with the current choice it is all that I can do.
    }
    cstr_sais(*sa->sa, CSTR_SLICE_CONST_CAST(*u_buf), &ALPHA);

    sa->next = next;

    free(u_buf); // We don't need this one any more.

    return sa;
}
static void free_sa(struct sa *sa)
{
    struct sa *next;
    for (; sa; sa = next)
    {
        next = sa->next;
        free(sa->sa);
        free(sa);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "%s genome reads", argv[0]);
        return 1;
    }
    const char *genome_fname = argv[1];
    const char *reads_fname = argv[2];

    // Initialise global alphabet.
    // NB! This assumes that we know the alphabet already and that it is "acgt" + $.
    // If we don't know it, we need to obtain it from the chromosomes.
    cstr_const_sslice letters = CSTR_SLICE_STRING0((const char *)"acgt");
    cstr_init_alphabet(&ALPHA, letters);

    struct fasta_records *genome = load_fasta_records(genome_fname);
    struct sa *suf_arrays = 0;
    for (struct fasta_record *fa_rec = fasta_records(genome); fa_rec; fa_rec = fa_rec->next)
    {
        suf_arrays = process_fasta_rec(fa_rec, suf_arrays);
    }

    struct fastq_iter fqiter;
    struct fastq_record fqrec;
    static const size_t CIGAR_BUF_SIZE = 2048;
    char cigarbuf[CIGAR_BUF_SIZE];

    FILE *fq = fopen(reads_fname, "r");
    if (!fq)
    {
        abort(); // I can always implement better checking another day...
    }
    init_fastq_iter(&fqiter, fq);

    while (next_fastq_record(&fqiter, &fqrec))
    {
        snprintf(cigarbuf, CIGAR_BUF_SIZE, "%lldM", fqrec.seq.len);

        for (struct sa *sa = suf_arrays; sa; sa = sa->next)
        {
            cstr_exact_matcher *m = cstr_sa_bsearch(*sa->sa, sa->x, fqrec.seq);
            for (long long i = cstr_exact_next_match(m); i != -1; i = cstr_exact_next_match(m))
            {
                print_sam_line(stdout, (const char *)fqrec.name.buf,
                               sa->chr_name, i, cigarbuf,
                               (const char *)fqrec.seq.buf);
            }
            cstr_free_exact_matcher(m);
        }
    }

    fclose(fq);
    free_sa(suf_arrays);
    free_fasta_records(genome);

    return 0;
}
