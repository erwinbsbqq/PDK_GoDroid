/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: hashtable.h
 *
 *  Description: Common hash table header file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/
#if defined(__ALI_TDS__)
#include <ali_rpc_util.h>
#else
#include "../../../include/alirpcng/ali_rpc_util.h"
#endif

/* Compute the number of buckets in ht */
#define NBUCKETS(ht)    ((ht)->nbuckets)

/* Compute the maximum entries given n buckets that we will tolerate, ~90% */
#define OVERLOADED(n)   ((n) - ((n) >> 3))


static void *defaultAllocTable(void *pool, Size_t size)
{
    return PR_Malloc(size);
}

static void defaultFreeTable(void *pool, void *item)
{
    PR_Free(item);
}

static HashEntry *defaultAllocEntry(void *pool, const void *key)
{
    return PR_Malloc(sizeof(HashEntry));
}

static void defaultFreeEntry(void *pool, HashEntry *he, Uint32 flag)
{
    if (flag == HT_FREE_ENTRY)
    {
        PR_Free(he);
    }
}

static Int32 defaultHashEnumerator(HashEntry *he, Int32 i)
{
    /** Print user key-value pair.
     *  Ie. key is a string and valus is an integer.
     *
        if (he) {
            PR_LOG("%d: <key: %s> <value: 0x%x>\n", i, (char *)he->key, *(int *)he->value);
        }
     */
    return HT_ENUMERATE_NEXT;
}

static HashAllocOps defaultHashAllocOps =
{
    defaultAllocTable, defaultFreeTable,
    defaultAllocEntry, defaultFreeEntry
};

enum { HASH_NUM_PRIMES = 28 }; //hash_num_primes

static const unsigned long hash_prime_list[HASH_NUM_PRIMES] =
{
    53ul,         97ul,         193ul,       389ul,       769ul,
    1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
    1610612741ul, 3221225473ul, 4294967291ul
};

static unsigned long hash_next_prime(unsigned long __n)
{
    const unsigned long *__first = hash_prime_list, *__middle = NULL;
    const unsigned long *__last = hash_prime_list + (int)HASH_NUM_PRIMES;
    long len = HASH_NUM_PRIMES, half = 0;

    while (len > 0)
    {
        half = len >> 1;
        __middle = __first + half;
        if (*__middle < __n)
        {
            __first = __middle;
            ++__first;
            len = len - half - 1;
        }
        else
        {
            len = half;
        }
    }

    return __first == __last ? *(__last - 1) : *__first;
}

HashTable *HashTableCreate(Uint32 n, HashFunction keyHash,
                           HashComparator keyCompare, HashComparator valueCompare,
                           const HashAllocOps *allocOps, void *allocPriv)
{
    HashTable *ht = NULL;
    Size_t nb = 0;

    n = hash_next_prime(n);

    if (!allocOps)
    {
        allocOps = &defaultHashAllocOps;
    }

    ht = (HashTable *)((*allocOps->allocTable)(allocPriv, sizeof * ht));
    if (!ht)
    {
        return 0;
    }

    memset(ht, 0, sizeof * ht);
    nb = n * sizeof(HashEntry *);
    ht->buckets = (HashEntry **)((*allocOps->allocTable)(allocPriv, nb));
    if (!ht->buckets)
    {
        (*allocOps->freeTable)(allocPriv, ht);
        return 0;
    }
    memset(ht->buckets, 0, nb);

    ht->nbuckets = n;
    ht->keyHash = keyHash;
    ht->keyCompare = keyCompare;
    ht->valueCompare = valueCompare;
    ht->allocOps = allocOps;
    ht->allocPriv = allocPriv;

    return ht;
}


void HashTableDestroy(HashTable *ht)
{
    Uint32 i = 0, n = 0;
    HashEntry *he = NULL, *next = NULL;
    const HashAllocOps *allocOps = ht->allocOps;
    void *allocPriv = ht->allocPriv;

    n = NBUCKETS(ht);
    for (i = 0; i < n; i++)
    {
        for (he = ht->buckets[i]; he; he = next)
        {
            next = he->next;
            (*allocOps->freeEntry)(allocPriv, he, HT_FREE_ENTRY);
        }
    }
#ifdef HASHDEBUG
    memset(ht->buckets, 0xDB, n * sizeof ht->buckets[0]);
#endif
    (*allocOps->freeTable)(allocPriv, ht->buckets);
#ifdef HASHDEBUG
    memset(ht, 0xDB, sizeof * ht);
#endif
    (*allocOps->freeTable)(allocPriv, ht);
}

HashEntry **HashTableRawLookup(HashTable *ht, HashNumber keyHash, const void *key)
{
    HashEntry *he = NULL, **hep = NULL, **hep0 = NULL;
    HashNumber h;

#ifdef HASHMETER
    ht->nlookups++;
#endif
    h = keyHash % ht->nbuckets;
    hep = hep0 = &ht->buckets[h];
    while ((he = *hep) != 0)
    {
        if (he->keyHash == keyHash && (*ht->keyCompare)(key, he->key))
        {
            /* Move to front of chain if not already there */
            if (hep != hep0)
            {
                *hep = he->next;
                he->next = *hep0;
                *hep0 = he;
            }
            return hep0;
        }
        hep = &he->next;
#ifdef HASHMETER
        ht->nsteps++;
#endif
    }

    return hep;
}

HashEntry **HashTableRawLookupConst(HashTable *ht, HashNumber keyHash,
                                    const void *key)
{
    HashEntry *he = NULL, **hep = NULL;
    HashNumber h;

#ifdef HASHMETER
    ht->nlookups++;
#endif
    h = keyHash % ht->nbuckets;
    hep = &ht->buckets[h];
    while ((he = *hep) != 0)
    {
        if (he->keyHash == keyHash && (*ht->keyCompare)(key, he->key))
        {
            break;
        }
        hep = &he->next;
#ifdef HASHMETER
        ht->nsteps++;
#endif
    }

    return hep;
}

HashEntry *HashTableRawAdd(HashTable *ht, HashEntry **hep,
                           HashNumber keyHash, const void *key, void *value)
{
    Uint32 i = 0, n = 0, nbuckets = 0;
    HashEntry *he = NULL, *next = NULL, **oldbuckets = NULL;
    Size_t nb = 0;

    /* Grow the table if it is overloaded */
    n = NBUCKETS(ht);
    if (ht->nentries >= OVERLOADED(n))
    {
        oldbuckets = ht->buckets;
        nbuckets = hash_next_prime(n + 1);
        nb = 2 * nbuckets * sizeof(HashEntry *);
        ht->buckets = (HashEntry **)((*ht->allocOps->allocTable)(ht->allocPriv, nb));
        if (!ht->buckets)
        {
            ht->buckets = oldbuckets;
            return 0;
        }
        memset(ht->buckets, 0, nb);
        ht->nbuckets = nbuckets;
#ifdef HASHMETER
        ht->ngrows++;
#endif

        for (i = 0; i < n; i++)
        {
            for (he = oldbuckets[i]; he; he = next)
            {
                next = he->next;
                hep = HashTableRawLookup(ht, he->keyHash, he->key);
                if (*hep != 0)
                {
                    PR_LOG("##### Oops! %s: Severe problem.\n", __func__);
                    return 0;
                }
                he->next = 0;
                *hep = he;
            }
        }
#ifdef HASHDEBUG
        memset(oldbuckets, 0xDB, n * sizeof oldbuckets[0]);
#endif
        (*ht->allocOps->freeTable)(ht->allocPriv, oldbuckets);
        hep = HashTableRawLookup(ht, keyHash, key);
    }

    /* Make a new key value entry */
    he = (*ht->allocOps->allocEntry)(ht->allocPriv, key);
    if (!he)
    {
        return 0;
    }

    he->keyHash = keyHash;
    he->key = key;
    he->value = value;
    he->next = *hep;
    *hep = he;
    ht->nentries++;

    return he;
}

HashEntry *HashTableAdd(HashTable *ht, const void *key, void *value)
{
    HashNumber keyHash;
    HashEntry *he = NULL, **hep = NULL;

    keyHash = (*ht->keyHash)(key);
    hep = HashTableRawLookup(ht, keyHash, key);

    if ((he = *hep) != 0)
    {
        /* Hit; see if values match */
        if ((*ht->valueCompare)(he->value, value))
        {
            /* key,value pair is already present in table */
            return he;
        }

        if (he->value)
        {
            (*ht->allocOps->freeEntry)(ht->allocPriv, he, HT_FREE_VALUE);
        }
        he->value = value;
        return he;
    }

    return HashTableRawAdd(ht, hep, keyHash, key, value);
}

void HashTableRawRemove(HashTable *ht, HashEntry **hep, HashEntry *he)
{
    *hep = he->next;
    (*ht->allocOps->freeEntry)(ht->allocPriv, he, HT_FREE_ENTRY);
    --ht->nentries;
}

Bool HashTableRemove(HashTable *ht, const void *key)
{
    HashNumber keyHash;
    HashEntry *he = NULL, **hep = NULL;

    keyHash = (*ht->keyHash)(key);
    hep = HashTableRawLookup(ht, keyHash, key);
    if ((he = *hep) == 0)
    {
        return False;
    }

    /* Hit; remove element */
    HashTableRawRemove(ht, hep, he);
    return True;
}

void *HashTableLookup(HashTable *ht, const void *key)
{
    HashNumber keyHash;
    HashEntry *he = NULL, **hep = NULL;

    keyHash = (*ht->keyHash)(key);
    hep = HashTableRawLookup(ht, keyHash, key);
    if ((he = *hep) != 0)
    {
        return he->value;
    }

    return 0;
}

void *HashTableLookupConst(HashTable *ht, const void *key)
{
    HashNumber keyHash;
    HashEntry *he = NULL, **hep = NULL;

    keyHash = (*ht->keyHash)(key);
    hep = HashTableRawLookupConst(ht, keyHash, key);
    if ((he = *hep) != 0)
    {
        return he->value;
    }

    return 0;
}

Int32 HashTableEnumerateEntries(HashTable *ht, HashEnumerator f)
{
    HashEntry *he = NULL, **hep = NULL;
    Uint32 i = 0, nbuckets = 0;
    int rv = 0, n = 0;
    HashEntry *todo = 0;

    nbuckets = NBUCKETS(ht);
    for (i = 0; i < nbuckets; i++)
    {
        hep = &ht->buckets[i];
        while ((he = *hep) != 0)
        {
            rv = (*f)(he, n);
            n++;
            if (rv & (HT_ENUMERATE_REMOVE | HT_ENUMERATE_UNHASH))
            {
                *hep = he->next;
                if (rv & HT_ENUMERATE_REMOVE)
                {
                    he->next = todo;
                    todo = he;
                }
            }
            else
            {
                hep = &he->next;
            }
            if (rv & HT_ENUMERATE_STOP)
            {
                goto out;
            }
        }
    }

out:
    hep = &todo;
    while ((he = *hep) != 0)
    {
        HashTableRawRemove(ht, hep, he);
    }
    return n;
}

#ifdef HASHMETER

#include <math.h>

void HashTableDumpMeter(HashTable *ht, HashEnumerator dump)
{
    double mean = 0, variance = 0;
    Uint32 nchains = 0, nbuckets = 0;
    Uint32 i = 0, n = 0, maxChain = 0, maxChainLen = 0;
    HashEntry *he = NULL;

    variance = 0;
    nchains = 0;
    maxChainLen = 0;
    nbuckets = NBUCKETS(ht);
    for (i = 0; i < nbuckets; i++)
    {
        he = ht->buckets[i];
        if (!he)
        {
            continue;
        }
        nchains++;
        for (n = 0; he; he = he->next)
        {
            n++;
        }
        variance += n * n;
        if (n > maxChainLen)
        {
            maxChainLen = n;
            maxChain = i;
        }
    }
    mean = (double)ht->nentries / nchains;
    variance = fabs(variance / nchains - mean * mean);

    PR_LOG("\nHash table statistics:\n");
    PR_LOG("     number of lookups: %u\n", ht->nlookups);
    PR_LOG("     number of entries: %u\n", ht->nentries);
    PR_LOG("       number of grows: %u\n", ht->ngrows);
    PR_LOG("     number of shrinks: %u\n", ht->nshrinks);
    PR_LOG("   mean steps per hash: %g\n", (double)ht->nsteps
           / ht->nlookups);
    PR_LOG("mean hash chain length: %g\n", mean);
    PR_LOG("    standard deviation: %g\n", sqrt(variance));
    PR_LOG(" max hash chain length: %u\n", maxChainLen);
    PR_LOG("        max hash chain: [%u]\n", maxChain);

    for (he = ht->buckets[maxChain], i = 0; he; he = he->next, i++)
        if ((*dump)(he, i) != HT_ENUMERATE_NEXT)
        {
            break;
        }
}
#endif /* HASHMETER */

Int32 HashTableDump(HashTable *ht, HashEnumerator dump)
{
    int count = 0;

    if (dump == 0)
    {
        dump = &defaultHashEnumerator;
    }
    count = HashTableEnumerateEntries(ht, dump);
#ifdef HASHMETER
    HashTableDumpMeter(ht, dump);
#endif
    return count;
}

HashNumber HashString(const void *key)
{
    HashNumber r = 0;
    const Uint8 *s = NULL;

    for (s = (const Uint8 *)key ; *s; ++s)
    {
        r = 5 * r + *s;
    }

    return r;
}

Int32 CompareStrings(const void *v1, const void *v2)
{
    return strcmp((const char *)v1, (const char *)v2) == 0;
}

Int32 CompareValues(const void *v1, const void *v2)
{
    return v1 == v2;
}

