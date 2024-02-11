# async

## Utility for transferinf UDP packets

**Features:**

* directories to scan (there may be several)
* directories to exclude from scanning (there may be several)
* scan level (one for all directories, 0 - only specified directory without attachments)
* minimum file size, all files are checked by default more than 1 byte.
* masks of file names allowed for comparison (case-independent)
* the size of the block used to read files, in the task this the size is referred to as S
* one of the available hashing algorithms (crc32, md5 - determine the specific options yourself), in the task this function is referred to as H

The result of the utility should be a list of full file paths with identical content, output to standard output. On one one file per line. Identical files must be in a row, in one group. The different groups are separated by an empty line.

> A mandatory feature of the utility is careful handling of disk input the conclusion. Each file can be represented as a list of blocks zeros.
___

## Options

* `sc`   *directories for scan (\"path\" ... \"path\")*
* `unsc` *directories for unscan (\"path\" ... \"path\")*
* `dpth` *dpth in tree of directories for scan (0..)*
* `msf`  *minimal size of file (1..)*
* `mask` *mask of file for scan (regexp)*
* `sb`   *size of block in file (bytes)*
* `hash` *algorithm of hash (md5, crc32)*

___

## Example

```shell
bayan --sc="path" "path" --unsc="path" "path" --dpth=2 --msf=5 --mask="*" --sb=20 --hash=md5
```
