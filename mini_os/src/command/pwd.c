#include "../header/Header.h"

void enqueuePath(DirectoryTree *tree, Directory *node, Queue *dirQueue) {
	if (node == NULL || node == tree->root) {
		if (node == tree->root) {
			char *rootName = strdup(node->name);
			if (!rootName) {
				fprintf(stderr, "Memory allocation failed\n");
				exit(1);
			}
			enqueue(dirQueue, rootName);
		}
		return;
	}

	enqueuePath(tree, node->parent, dirQueue);

	char *nameCopy = strdup(node->name);
	if (nameCopy == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}
	enqueue(dirQueue, nameCopy);
}

void printQueuePath(Queue *queue) {
	while (!isEmpty(queue)) {
		char *name = dequeue(queue);
		printf("/%s", name);
		free(name);
	}
	printf("\n");
}

void printPath(DirectoryTree *dirTree, Queue *dirQueue) {
	DirectoryTree *current = dirTree->current;
	if (current == dirTree->root) {
		printf("/\n");
	} else {
		enqueuePath(dirTree, current, dirQueue);
		printQueuePath(dirQueue);
	}
}

int pwd(DirectoryTree *dirTree, Queue *Queue, char *option) {
	if (dirTree == NULL) {
		printf("The current directory information could not be found.\n");
		return false;
	}

	if (option == NULL || strcmp(option, "-") == 0 || strcmp(option, "--") == 0 || strcmp(option, "-p") == 0) {
		printPath(dirTree, Queue);
	} else if (strcmp(option, "--help") == 0) {
		printf("pwd: pwd [-LP]\n");
		printf("  Print the name of the current working directory.\n\n");
		printf("  Options:\n");
		printf("    -L\t print the value of $PWD if it names the current working\n");
		printf("  \t directory\n");
		printf("    -P\t print the physical directory, without any symbolic links\n\n");
		printf("  By default, 'pwd' behaves as if '-L' were specified.\n\n");
		printf("  Exit status:\n");
		printf("  Returns 0 unless an invalid option is given or the current directory\n");
		printf("  cannot be read.\n");
	} else if (strcmp(option, "-LP") == 0 || strcmp(option, "-PL") == 0) {
		printPath(dirTree, Queue);
	} else {
		printf("-bash: pwd: %.2s: invalid option\n", option);
		printf("pwd: usage: pwd [-LP]\n");
		return false;
	}
	return true;
}
