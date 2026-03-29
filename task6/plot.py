import csv
import matplotlib.pyplot as plt
import sys

def main():
    csv_filename = "histogram_data.csv"
    bin_starts = []
    file_counts = []

    # Read the aggregated data
    try:
        with open(csv_filename, mode='r') as file:
            reader = csv.DictReader(file)
            for row in reader:
                bin_starts.append(str(row['BinStart']))
                file_counts.append(int(row['FileCount']))
    except FileNotFoundError:
        print(f"Error: Could not find '{csv_filename}'. Ensure the C++ program has run successfully.")
        sys.exit(1)

    if not bin_starts:
        print("Error: The CSV file is empty.")
        sys.exit(1)

    # Generate the bar chart
    plt.figure(figsize=(12, 6))
    plt.bar(bin_starts, file_counts, color='#4C72B0', edgecolor='black', width=1.0)

    plt.title('File Size Histogram', fontsize=14, fontweight='bold')
    plt.xlabel('Bin Start Range (Bytes)', fontsize=12)
    plt.ylabel('Number of Files', fontsize=12)

    # Format the x-axis for readability
    plt.xticks(rotation=45, ha='right', fontsize=8)
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()

    # Display the plot
    plt.show()

if __name__ == "__main__":
    main()