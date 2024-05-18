def edit_file(filename):
    # Open the file for reading
    with open(filename, 'r') as file:
        lines = file.readlines()

    # Open the file for writing
    with open(filename, 'w') as file:
        for line in lines:
            # Check if the line starts with a tab
            if not line.startswith('\t'):
                # Find the index of the comma
                comma_index = line.find(',')

                # If comma exists, remove characters from comma to newline
                if comma_index != -1:
                    line = line[:comma_index] + '\n'  # Keep the newline character

            # Write the modified line to the file
            file.write(line)

    return len(lines)  # Return the number of lines

def main():
    while True:
        filename = input("\nEnter filepath to hashify: ")
        if filename == "STOP":
                break
                
        num_lines = edit_file(filename)
        print("Files successfully hashified!")
        print("Hashes in file: ", num_lines)
            
    print("Have a nice day! :D")    


if __name__ == "__main__":
    main()
    