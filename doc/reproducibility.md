# Reproducibility of OpenTRIM results {#reproducibility}

Monte-Carlo programs use pseudo-random number generator (RNG) algorithms which produce a very long sequence of numbers that are seemingly random and uncorrelated.

The RNG seed sets effectively the point in this sequence where a specific Monte-Carlo run starts to read numbers.

Thus, two runs of the same Monte-Carlo calculation, starting from the same seed will produce exactly the same result. This reproducibility reveals the pseudo-random character of these calculations. At the same time, it helps programmers correct bugs in the code, which are revealed when the reproducibility is broken.

OpenTRIM should always produce exactly the same results when running the same simulation, with the same seed and for the same number of ion histories.

A detail that has to be noted is that, in order for the results to be reproducible, OpenTRIM must also run **with the same number of threads**.
Thus, for example, a single-thread run of 1000 ions will not produce the same results when 2 threads are used that simulate 500 ions each.

This is due to the way RNGs are used in multi threaded applications. Each thread makes a very long "jump" in the random number sequence relative to the seed and starts reading numbers there. Thus, each thread has effectively its own number sequence. RNG algorithms provide a way to do this reproducibly, ensuring minimal correlation between thread sequences. However, the number of threads changes the actual pseudo-random sequence that the calculation uses.

OpenTRIM uses the Xoshiro256+ RNG (https://prng.di.unimi.it/) with a period of \f$ 2^{256} - 1\f$. 

\sa \ref RNG