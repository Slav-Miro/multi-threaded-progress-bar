# Multithreaded progress bars

 A program meant to show multiple progress bars ("Tasks") and their multithreaded implementation.

 ## Motivation
 I recently covered threads and multithreaded programming in my operating systems class. I really like the topic and wanted to do a little more to develop my learning so I developed this. While simple, it also uses ANSI escape codes for formatting and text coloring.

 ## Use
 Simply just run ```make``` and run using ```./multithreaded_progress```.
 OR
 Compile using your favorite compiler. If need be, use the respective pthread flag.

 **Clang**
 ```clang main.c -o multithreaded_progress -lpthread```
 **GCC**
 ```gcc main.c -o multithreaded_progress -lpthread```

 Not supported for Windows as Windows doesn't use the ```<pthreads.h>``` library, but can be used on windows systems through the Windows Subsystem for Linux (WSL).

 ## Program breakdown
 If you stumbled across this page for your own learning purposes, it might be beneficial to go over this section and pick out what you might need.

Let's go over the definitions first:
```
#define ESCAPE "\x1B[%d;%dH%s"
#define TASKS 20
#define COUNT 80
```
- ```ESCAPE```- An ANSI escape code combination that allows us to pass an X and Y to move the cursor as we please.[Link to more ANSI codes.](https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b)
- ```TASKS```- Determines how many tasks to create and therefore threads.
- ```COUNT```- Size of the progress bar

As per usual, we create an array of threads using ```pthread_t thread_array[TASKS];``` so we have some way to keep track and access them.

We have a structure called ```data_struct``` to store information about specific printing information. Since each thread moves at its own rate, we use this structure to store where the cursor should be when printing.

We also need to create a mutex lock to avoid deadlocks when writing to stdout. This is done through: ```pthread_mutex_t stdout_mutex_lock = PTHREAD_MUTEX_INITIALIZER;```

### The ```main``` function
To make our workspace clean, we call ```printf("\033[2J\033[H");``` to clear the terminal. 
We then allocate an array on the heap of our data structure type using:
```data_struct * data_pointer = malloc(sizeof(data_struct) * TASKS);```
It should be noted that we should always have some error checking whenever we allocate anything on the heap.

We initialize some of our data through our first loop: 
```
for (int i = 0 ; i < TASKS; i++){
    data_pointer[i].col = 15;
    data_pointer[i].row = i + 3;
    data_pointer[i].task_time =(rand() % 10) + 1;
    data_pointer[i].count = 0;

    //Create thread and assign it's respective data
    pthread_create(&thread_array[i], NULL, thread_handler, &data_pointer[i]);
}
```
Without going too in depth with ```pthread_create```, I will quickly take the time to explain the parameters:
- ```&thread_array[i]```- Direct address of the thread we want to create and update
- ```thread_handler```- A function pointer which the thread will run and execute it's logic
- ```&data_pointer[i]```- Address of the data to pass to our function pointer. Since function pointers take a ```void *``` parameter, we will unwrap the data later. This implementation also allows us to pass multiple types of data using only one parameter which is pretty cool.

We then need a seperate loop to join the threads:
```
for (int i = 0 ; i < TASKS; i++){
    pthread_join(thread_array[i], NULL);
}
```
It is important to call this __after__ we create our threads to allow the OS ample time to create a thread and not run into any issues.

### The ```void * thread_handler``` function
This is a function pointer definition that will handle our thread logic. If you would like more information about function pointers, feel free to read the [Wikipedia page](https://en.wikipedia.org/wiki/Function_pointer) for function pointers.

One thing to note about using threads, we typically want to protect shared data to avoid race conditions. You might be thinking, "Well we don't seem to have shared data since each thread has it's own reference to the data." The threads are actually sharing the stdout buffer. Because of this, we need to synchronize the threads to allow when they have access to the buffer. Because of this, we have to lock the mutex whenever we need to print and this is done using ```pthread_mutex_lock``` and ```pthread_mutex_unlock```. 

```
pthread_mutex_lock(&stdout_mutex_lock);     //Lock
data->task_id =  pthread_self() % 10000;
printf("\033[%d;%dHTASK ID: %4d: ",data->row, 1,data->task_id);
for (int i = 0 ; i < COUNT - 1; i++){
    printf("\033[38;5;226m-");
}
pthread_mutex_unlock(&stdout_mutex_lock);   //Unlock
```

The ANSI codes are just responsible for formatting and just setting up our workspace and fill it with "eyecandy" so we don't work with a blank screen.

Now, the juice of the ```thread_handler``` function:
```
while (data->count < COUNT){
    pthread_mutex_lock(&stdout_mutex_lock);
    printf("\033[38;5;46m"ESCAPE, data->row, data->col , "#");
    printf("\033[38;5;255m");
    data->col++;
    data->count++;
    fflush(stdout);
    data->task_time =(rand() % 10) + 1;
    if (data->task_time == 0) { data->task_time = 5; }
    pthread_mutex_unlock(&stdout_mutex_lock);
    usleep(10000 * data->task_time);
}
```

This whole ```while``` loop is what keeps our thread alive and runs until our task is finished. Once it is done, the OS just kills the thread and frees it. The whole program won't finish until all the threads are done so we don't have to worry about it exiting early or anything like that.

In really quick terms, this loop moves the cursor by one column after the task's ```task_time``` has passed and increments the count. The ANSI codes overwrite whatever character it is placed on so we can print ```#``` in the respective X and Y coordinates of the data structure. 