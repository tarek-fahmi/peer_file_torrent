#!/bin/bash

num_passes=0

# Define function to run tests for a specific part
run_test() {
    local test_name="${1}_test_${2}"

    local input_file="./${1}/$test_name/data.in"
    local expected_output_file="./${1}/${1}_test_${2}/data.out"

    local test_command=$(head -n 1 "$input_file")  # Read the first line as the test command

    printf "\n\tStart $test_command...\n"

    local run_function=$(head -n 1 "$input_file")
    local actual_output=$(eval "$run_function")

    local end_time=$(date +%s.%N)  # End time of the test
    local duration=$(echo "$end_time - $start_time" | bc)
    
    local current_time=$(date +"%T")

    if diff -q "$expected_output_file" <(echo "$actual_output") >/dev/null; then
        printf "\tPASSED at $current_time!\n"
        printf "\t Ran for $duration s\n"
        return 0
    else
        printf "\t$dir FAILED at $current_time\n"
        printf "\t Ran for $duration s\n"
        return 1
    fi
}

check_sec_input() {
    local sec_choice="$1"
    if [[ "$sec_choice" != "merkle" && "$sec_choice" != "chunk" && "$sec_choice" != "package" ]]; then
        printf "Invalid section name entered! \n"
        sleep 1.5
        return 1
    fi
    return 0
}

# Function to run all tests under a directory
run_all_tests() {
    local section="$1"
    if ! check_sec_input "$section"; then
        return
    fi

    local num_tests=$(ls -l | grep -c ^-)

    for ((i=0; i<num_tests; i++)); do
        if run_test "$section" "$i"; then
            num_passes=$((num_passes + 1))
        fi
    done
}

main() {
    printf $"Welcome to Proj. ByteTide: Package + Merkle-Tree Testing Controller!\n\n"

    local running=true
    while $running; do
        echo "Choose which tests to run:"
        echo "All:            '1'"
        echo "Section tests:  '2'"
        echo "Specific test:  '3'"
        echo "Exit controller:'4'"

        
        printf $"\nPick your testing option: \n:> "
        read opt 

        case $opt in
            3)
                printf $"\n[Mono-test Mode]\n\tTest Options:\n\tchunk [1-2]\n\tpackage [1-2]\n\tmerkle [1-4]\n"
                printf $"\tInput your choice in this format: '{section_name} {test_num}'\n\t:> "
                printf $"\t"
                read part

                printf $"\tEnter test number: \n\t:> "
                read test_number

                run_test "$part" "$test_number"
                ;;
            1)
                run_all_tests "chunk"
                run_all_tests "merkle"
                run_all_tests "package"
                ;;
            2)
                printf $"\n[Section Mode]\n\tEnter section name: ('package', 'chunk', 'merkle'): \n\t:> " 
                read sec_input
                local sec_choice=$ echo "$sec_input" | tr '[:upper:]' '[:lower:]'
                run_all_tests "$sec_choice"
                ;;
            4)
                echo $"\nExiting test controller... Have a nice day!"
                running=false
                ;;
            *)
                echo $"\nInvalid option selected."
                ;;
        esac
    done
}

main
