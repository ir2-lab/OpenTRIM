set(physics_datasets
    /tally/damage_events/Implantations
    /tally/damage_events/Recombinations
    /tally/damage_events/Replacements
    /tally/damage_events/Vacancies
    /tally/energy_deposition/Ionization
    /tally/energy_deposition/Lattice
    /tally/energy_deposition/Lost
    /tally/energy_deposition/Stored
    /tally/ion_stat/Collisions
    /tally/ion_stat/Flight_path
    /tally/ion_stat/Lost
    /tally/pka_damage/Pka
    /tally/pka_damage/Pka_energy
    /tally/pka_damage/Tdam
    /tally/pka_damage/Tdam_LSS
    /tally/pka_damage/Vnrt
    /tally/pka_damage/Vnrt_LSS
    /tally/totals/data
)

set(error_datasets
    /tally/damage_events/Implantations_sem
    /tally/damage_events/Recombinations_sem
    /tally/damage_events/Replacements_sem
    /tally/damage_events/Vacancies_sem
    /tally/energy_deposition/Ionization_sem
    /tally/energy_deposition/Lattice_sem
    /tally/energy_deposition/Lost_sem
    /tally/energy_deposition/Stored_sem
    /tally/ion_stat/Collisions_sem
    /tally/ion_stat/Flight_path_sem
    /tally/ion_stat/Lost_sem
    /tally/pka_damage/Pka_sem
    /tally/pka_damage/Pka_energy_sem
    /tally/pka_damage/Tdam_sem
    /tally/pka_damage/Tdam_LSS_sem
    /tally/pka_damage/Vnrt_sem
    /tally/pka_damage/Vnrt_LSS_sem
    /tally/totals/data_sem
)

foreach(target IN LISTS physics_datasets)
    execute_process(
        COMMAND "${H5DIFF}" -v --relative=1e-9 "${PRODUCED}" "${TEMPLATE}" "${target}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(result)
        message(FATAL_ERROR "h5diff mismatch in ${target}:\n${output}${error}")
    endif()
endforeach()
foreach(target IN LISTS error_datasets)
    execute_process(
        COMMAND "${H5DIFF}" -v --relative=1e-6 "${PRODUCED}" "${TEMPLATE}" "${target}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(result)
        message(FATAL_ERROR "h5diff mismatch in ${target}:\n${output}${error}")
    endif()
endforeach()
