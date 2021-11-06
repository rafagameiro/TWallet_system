/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */

#include <tee_internal_api.h>
#include <string.h>

#include <split.h>

/*
 * Method that based on a character passed as argument,
 * it will split the string into smaller substrings
 */
int split (const char *txt, char delim, char ***tokens)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = TEE_Malloc (count * sizeof (int), 0);
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = TEE_Malloc (count * sizeof (char *), 0);
    t = tklen;
    p = *arr++ = TEE_Malloc ((*(t++) + 1) * sizeof (char *), 0);
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = TEE_Malloc ((*(t++) + 1) * sizeof (char *), 0);
            txt++;
        }
        else *p++ = *txt++;
    }
    TEE_Free (tklen);
    return count;
}
