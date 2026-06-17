# COMP2421 – Data Structures Project 2

**Student Name:** Emam Samara
**Student ID:** 1220022
**Section:** 1

## Project Overview

This project implements a Building Management System using two fundamental data structures:

1. **AVL Tree**
2. **Hash Table**

The system allows efficient storage, retrieval, updating, deletion, and management of building records. Data can be loaded from files, manipulated through interactive menus, and saved back to files.

---

## Building Record Format

Each building contains:

* Building Name
* Building Number
* Address
* Number of Apartments
* Establishment Year
* Paid Fees Status (yes/no)

Example:

```text
Al Noor:200:University Street:30:2018:no
```

---

# Part 1: AVL Tree

The AVL tree stores building records sorted alphabetically by building name.

### Supported Operations

* Load data from `info.txt`
* Insert a new building
* Search for a building
* Update building information
* Delete a building
* List all buildings alphabetically
* List buildings with apartments greater than a given value
* List buildings that have not paid fees
* Save AVL data to `hash.txt`

### AVL Features

* Self-balancing binary search tree
* Maintains O(log n) search, insertion, and deletion
* Uses single and double rotations:

  * Left Rotation
  * Right Rotation
  * Left-Right Rotation
  * Right-Left Rotation

---

# Part 2: Hash Table

The hash table stores building records using open addressing with linear probing.

### Hash Function

The hash value is computed using only the first four characters of the building name as required by the project specification.

### Collision Handling

* Open Addressing
* Linear Probing

### Supported Operations

* Load data from `hash.txt`
* Print the entire hash table
* Display table size
* Display load factor
* Insert a new building
* Search for a building
* Delete a building
* Save the hash table back to `hash.txt`

### Hash Entry States

* EMPTY
* OCCUPIED
* DELETED

---

# Input Files

### info.txt

Contains the initial building records used to create the AVL tree.

### hash.txt

Stores building records used by the hash table and can be generated from AVL tree data.

---

# Program Menus

## Main Menu

```text
1. AVL Tree Section
2. Hash Table Section
3. Save Hash Table and Exit
4. Exit Without Saving Hash Table
```

## AVL Menu

```text
1. Load info.txt and create AVL tree
2. Insert a new building
3. Search for a building and optionally update it
4. List all buildings alphabetically
5. List buildings whose apartments are greater than input
6. List buildings that have not paid fees
7. Delete a building
8. Save AVL tree to hash.txt
9. Save AVL tree and open Hash Table Menu
10. Return to Main Menu
```

## Hash Table Menu

```text
1. Load hash table from hash.txt
2. Print full hash table including empty spots
3. Print table size and load factor
4. Insert a new building
5. Search for a building and print collision count
6. Delete a building
7. Save hash table to hash.txt
8. Return to Main Menu
9. Save and Exit
```

---

# File Handling

The program automatically locates:

* `info.txt`
* `hash.txt`

relative to the executable location, allowing execution from different working directories on both Windows and Linux.

---

# Compilation

### Windows (MinGW)

```bash
gcc -Wall -Wextra -Wpedantic -std=c11 P2_1220022_EmamSamara_1.c -o project2.exe
```

### Linux (GCC)

```bash
gcc -Wall -Wextra -Wpedantic -std=c11 P2_1220022_EmamSamara_1.c -o project2
```

---

# Memory Management

The project dynamically allocates memory for:

* AVL tree nodes
* Hash table entries

All allocated memory is properly released before program termination.

---

# Data Structures Used

* AVL Tree
* Hash Table
* Structures
* Dynamic Memory Allocation
* File Processing
* Open Addressing
* Linear Probing

---

# Author

**Emam Samara**
Student ID: **1220022**
COMP2421 – Data Structures Project 2
