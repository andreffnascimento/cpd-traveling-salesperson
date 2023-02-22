<h1 align="center">Traveling Salesperson • Branch & Bound Algorithm</h1>
<h2 align="center">Parallel and Distributed Computing, 2022/2023, @IST</h2>

<p align="center">
  <a href="#about">About</a> •
  <a href="#technologies">Technologies</a> •
  <a href="#installation">Installation</a> •
  <a href="#execution">Execution</a> •
  <a href="#license">License</a> •
  <a href="#authors">Authors</a>
</p>



# About

This is a class project implemented in the context of the Parallel and Distributed Computing course, 2022/2023, at Instituto Superior Técnico. Its purpose is to give students a hands-on experience in parallel programming on both shared-memory and distributed-memory systems, using OpenMP and MPI, respectively. 

The idea of this assignment is to write a sequential and two parallel implementations of a program that computes the shortest route to visit a given set of cities, the well-known traveling salesperson problem. This optimization problem falls in the NP-Hard complexity class, meaning that all known solutions to solve it exactly imply an exhaustive search over all possible sequences. 

<br>



# Technologies
- [C](https://en.wikipedia.org/wiki/C_(programming_language))
- [OpenMP](https://www.openmp.org/)

<br>



# Installation
This section shows the installation steps for a generic linux-based system (fully tested on the [Kali Linux](https://www.kali.org/) distribution).

- **Install** the technologies required by the program
```
sudo apt update && sudo apt upgrade
sudo apt install build-essential
```

- **Build** the executable file
```
make clean
make build
```

<br>



# Execution
This section explains how to run the project:

- **Shell** command
```
./tsp <cities_file> <max_value>
```

<br>



# License
This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

<br>



# Authors
| Name               | GitHub ID         | Email                                 |
| :----------------- | :---------------- | ------------------------------------: |
| André Nascimento   | ArckenimuZ        | andreffnascimento@tecnico.ulisboa.pt  |
