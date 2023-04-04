import matplotlib.pyplot as plt
import re
import os


def get_results(folder_path: str):
    res = []
    contents = None
    pattern = re.compile(r"\[(Succ|Fail)\]\s+([\w-]+)\.in\s+\((\d+\.\d+s)\)")
    for filename in os.listdir(folder_path):
        with open(os.path.join(folder_path, filename), "r") as file:
            contents = file.read()

        matches = re.findall(pattern, contents)

        names = []
        values = []

        for match in matches:
            names.append(re.findall("[a-zA-Z0-9]+", match[1])[0])
            values.append(float(match[2][:-1]))
    
        res.append((filename.split(".")[0], names, values))
    return res

def configure_plot(title, xlabel, ylabel):
    # plt.figure().set_size_inches(10,10)
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)


def generate_plot(x, y, label):
    plt.plot(x, y, '.-', label=label, linewidth=1)
    # plt.xticks(rotation=90)

def show_plot():
    plt.legend()
    plt.show()

def main():
    results = get_results("../../omp_measurements/results")
    configure_plot("Time comparison between different TSP implementations", "Test name", "Time(s)")
    for (test_name, x, y) in results:
        generate_plot(x, y, test_name)
    show_plot()

if __name__ == '__main__':
    main()