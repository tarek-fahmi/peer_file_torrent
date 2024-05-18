#!/bin/bash

num_passes=0

# Define function to run tests for a specific part
run_test() {
    local part="$1"
    local test_num="$2"
    local test_dir="./tests/${part}/${part}_test_${test_num}"
    local input_file="${test_dir}/data.in"
    local executable="./bin/pkg_main"
    local expected_output_file="${test_dir}/data.out"

    if [ ! -f "$executable" ]; then
        printf "Please place the executable into the bin and restart the controller. Thanks!\n"
        exit
    fi
    if [ ! -f "$input_file" ]; then
        if ["input_file" == "./bin/pkg_main"]; then
            printf "pkgmain missing..."
            exit
        fi
        printf "%s does not exist!\n" "$input_file"
        return 1
    elif [ ! -f "$expected_output_file" ]; then
        printf "%s does not exist!\n" "$expected_output_file"
        return 1
    fi

    local test_name=$(head -n 1 "$input_file")  # Read the first line as the test name
    printf "\t\n$test_name ahhh\n"
    local start_time=$(date +%s.%N)  # Start time of the test

    # Run all commands from the second line onwards
    local actual_output=""
    while IFS= read -r line; do
        actual_output+=$(eval "$line\n")
    done < <(tail -n +2 "$input_file")

    local end_time=$(date +%s.%N)  # End time of the test
    local duration=$(echo "$end_time - $start_time")

    local current_time=$(date +"%T")

    if diff -q "$expected_output_file" <(echo -e "$actual_output") >/dev/null; then
        printf "\tPASSED at $current_time!\n"
        printf "\tRan for $duration s\n"
        return 0
    else
        printf "\t$part test $test_num FAILED at $current_time\n"
        return 1
    fi
}

check_sec_input() {
    local sec_choice="$1"
    if [[ "$sec_choice" != "merkletree" && "$sec_choice" != "chunk" && "$sec_choice" != "package" ]]; then
        printf "Invalid section name entered! \n"
        sleep 1.5
        return 1
    fi
    return 0
}

# Function to run all tests under a directory
run_all_tests() {
    local section="$1"
    if ! check_sec_input "$1"; then
        return 1
    fi

    local num_tests=$(find "./tests/${section}/" -mindepth 1 -maxdepth 1 -type d | wc -l)

    for ((i=1; i<=num_tests; i++)); do
        if run_test "$section" "$i"; then
            num_passes=$((num_passes + 1))
        fi
    done
}

main() {
    printf "Welcome to Proj. ByteTide: Package + Merkle-Tree Testing Controller!\n\n"

    local running=true
    chmod +x ./bin/pkg_main
    while $running; do
        echo "Choose which tests to run:"
        echo "All:            '1'"
        echo "Section tests:  '2'"
        echo "Specific test:  '3'"
        echo "Exit controller:'4'"

        printf "\nPick your testing option: \n:> "
        read opt 

        case $opt in
            3)
                printf "\n[Mono-Test Mode]\n\tTest Options:\n"
                printf "chunk      [1-2]\n"
                printf "package    [1-2]\n"
                printf "merkletree [1-6]\n"
                printf "Choose a test to run: '{section_name} {test_num}'\n\t:> "
                read part test_number
                run_test "$part" "$test_number"
                ;;
            1)
                printf "\n[All Tests Mode]\n\tCommencing...\n"
                printf "\n\tTesting Chunk..."
                run_all_tests "chunk"
                printf "\n\tTesting Merkle Tree..."
                run_all_tests "merkletree"
                printf "\n\tTesting Package..."
                run_all_tests "package"
                printf $"\n\n\tPassed $num_passes/10 tests\n\n"
                ;;
            2)
                printf "\n[Section Mode]\n\tEnter section name: ('package', 'chunk', 'merkletree'): \n\t:> " 
                read sec_input
                local sec_choice=$(echo "$sec_input" | tr '[:upper:]' '[:lower:]')
                run_all_tests "$sec_choice"
                ;;
            4)
                printf "\nExiting test controller... Have a nice day!\n"
                running=false
                ;;
            *)
                echo "\nInvalid option selected.\n"
                ;;
        esac
    done
}

main
