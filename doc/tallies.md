# Tallies & events {#tallies}

The results of the simulation are tally tables and events.

## Tallies

A tally in the Monte-Carlo method means a score table were the occurrences of
certain events is counted (tallied). In the end, we divide the score by the total number of trials to obtain the probability of an event.

OpenTRIM stores currently \f$N_{tally}=19\f$ standard tally tables for various quantities of interest.
They are saved in the \ref out_file "HDF5 output file" under the path `/tally`.

Each element in the tally tables is a mean value over all histories. Specifically, if \f$x_i\f$ is the contribution to the table entry \f$x\f$ from the \f$i\f$-th ion history, then  the tally table will contain the mean value
\f[
\bar{x} = \frac{1}{N_h} \sum_i {x_i}
\f]

For each tally table, a second table of equal dimensions is stored, which holds the standard error of the mean (SEM). This has the same name plus the ending `_sem`. E.g., for the table `/tally/damage/Tdam` there is also `/tally/damage/Tdam_sem`. The SEM of a tally table entry, \f$\sigma_{\bar{x}}\f$, is calculated as
\f[
\sigma_{\bar{x}}^2 = \frac{1}{N_h(N_h-1)} \sum_i { (x_i - \bar{x})^2 }=
\frac{\bar{{x^2}} - \bar{x}^2}{N_h-1}
\f]

The table stored in `/tally/totals/data` holds total scores and has dimensions \f$[N_{tally} \times N_{atoms}]\f$, where \f$N_{atoms}\f$ counts all the different atoms present in the problem including the projectile. Thus, each element corresponds to the total score of a given quantity for each atom.

All other tables have dimensions \f$[N_{atoms} \times N_{cells}]\f$. Thus, the value of element \f$(i,j)\f$ corresponds to the score of a given quantity for the \f$i-\f$th atom in \f$j-\f$th cell. For example, the element \f$(0,1)\f$ in table `/tally/energy_deposition/Ionization` holds the contribution per ion history to ionization from the atom of index 0 (this is the projectile) in the cell of index 1.

\see tally

## Events

\todo Write the events documentation

\sa Event, event, pka_event, exit_event, event_stream 