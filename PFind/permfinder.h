/*******************************************************************************
 * Name        : permfinder.h
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 03/16/21
 * Description : Finds files with a specified set of permissions. 
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
 
int perm_checker(char *perm);
int perm_finder(char *path, char *permissions);
int perm_cmp(struct stat *statbuf, char *permissions);