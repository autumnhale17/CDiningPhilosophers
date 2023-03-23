// Autumn Hale
// CSCI 4100
// Programming Assignment 4
// Source file for Dining Philosophers simulation. Simulates using threads, locks, and condition variables.

#include "dining_room.h"

// Tests to see if it is safe for the philosopher with the ID provided to eat
// If it is safe, it changes the philosopher's state to EATING and returns
// true, if not it returns false.
// This function must only be called after table_lock has been acquired
int test_phil(struct dining_room *room,  int phil)
{
  // If the state for phil. is HUNGRY & neither neighbors EATING, true
  if (room->phil_state[phil] == HUNGRY && 
    room->phil_state[left_neighbor(room, phil)] != EATING &&
    room->phil_state[right_neighbor(room, phil)] != EATING) {
      return 1;
    }
  else {
    return 0;
  }
}

// Starts a dining philosopher simulation
void run_simulation(struct dining_room *room)
{
  // Display headings & phil. initial states
  display_headings(room);
  display_states(room);

  // Starting the threads for each phil. depending on phil. no. entered
  for (int i = 0; i < room->num_phils; i++) {
  pthread_create(&room->phil_threads[i], NULL, start_philosopher, &room->phil_args[i]);
  }

  // Waiting for each phil. thread to complete depending on cycles entered
  for (int i = 0; i < room->num_cycles; i++) {
  pthread_join(room->phil_threads[i], NULL);
  }
  
  
}

// Simulates a philosopher picking up forks
void grab_forks(struct dining_room *room, int phil)
{
  // Acquire the table lock 
  pthread_mutex_lock(&room->table_lock);
  
  // Setting the state of phil. to HUNGRY
  room->phil_state[phil] = HUNGRY;

  // Display the current states of the phils.
  display_states(room);

  // Wait on condition variable for phil. to return true
  while(test_phil(room, phil) == 0) {
    pthread_cond_wait(&room->safe_to_eat[phil], &room->table_lock);
  }

  // Set the state for phil. to EATING
  room->phil_state[phil] = EATING;

  // Display the current states of the phils.
  display_states(room);

  // Release the table lock 
  pthread_mutex_unlock(&room->table_lock);
}

// Simulates a philosopher putting down forks
void release_forks(struct dining_room *room, int phil )
{
  // Acquire the table lock 
  pthread_mutex_lock(&room->table_lock);

  // Setting the state of phil. to THINKING
  room->phil_state[phil] = THINKING;

  // Display the current states of the phils.
  display_states(room);

  // Test to see if it is safe for each of phil.'s neighbors to eat
  if(test_phil(room, left_neighbor(room, phil)) == 1) {
    // Notify the phil. waiting
    pthread_cond_signal(&room->safe_to_eat[left_neighbor(room, phil)]);
  }

  if(test_phil(room, right_neighbor(room, phil)) == 1) {
    // Notify the phil. waiting
    pthread_cond_signal(&room->safe_to_eat[right_neighbor(room, phil)]);
  }

  // Release the table lock 
  pthread_mutex_unlock(&room->table_lock);
}

// Displays the headings for the state change table
void display_headings(struct dining_room *room)
{
  int phil;
  for(phil = 0; phil < room->num_phils; phil++)
    printf("PHIL %-5d", phil);
  printf("\n");
}

// Displays the current state of all philosophers.
// This should only be called if table_lock has been acquired
void display_states(struct dining_room *room)
{
  int phil;
  for(phil = 0; phil < room->num_phils; phil++) {
      switch(room->phil_state[phil]) {
      case THINKING: printf("%-10s", "THINKING"); break;
      case HUNGRY: printf("%-10s", "HUNGRY"); break;
      case EATING: printf("%-10s", "EATING"); break;
      default: printf("%-10s", "CONFUSED");
      }
    }
  printf("\n");
  fflush(stdout);
}

// Gets the ID of the left neighbor of phil
int left_neighbor(struct dining_room *room, int phil )
{
  return phil == room->num_phils - 1? 0 : phil + 1;
}

// Gets the ID of the right neighbor of phil
int right_neighbor(struct dining_room *room, int phil )
{
  return phil == 0 ? room->num_phils - 1 : phil - 1;
}

// Simulates a philosopher thinking
void think( )
{
  unsigned int seed = time(0);
  srand(seed);
  usleep(rand() % 500000);
}

// Simulates a philosopher eating
void eat( )
{
  unsigned int seed = time(0);
  srand(seed);
  usleep(rand() % 500000);
}

// Code for a philosopher thread to run.
// This function should be used when creating a new philosopher thread
void *start_philosopher( void *the_args )
{
  struct p_args *args = (struct p_args *)the_args;  
  int i;
  for(i = 0; i < args->num_cycles; i++) {
    think();
    grab_forks(args->room, args->phil_num);
    eat();
    release_forks(args->room, args->phil_num);
  }
  return NULL;
}

// Initializes a dining_room structure
// num_phils is the number of philosophers
// num_cycles is the number of times each philosopher tries to eat
void init_dining_room(struct dining_room *room, int num_phils, int num_cycles )
{
  // Check for valid number of philosophers
  if(num_phils > MAX_PHILS || num_phils < 1) {
    fprintf(stderr, "Error: invalid number of philosophers");
    exit(1);
  }

  // Check for valid number of cycles
  if(num_cycles < 1) {
    fprintf(stderr, "Error: invalid number of philosophers");
    exit(1);
  }
  
  // Initialize the simulation parameters
  room->num_phils = num_phils;
  room->num_cycles = num_cycles;
  
  // Initialize the table lock
  pthread_mutex_init(&room->table_lock, NULL);
  
  // Initialize the data in the arrays
  int phil;
  for(phil = 0; phil < num_phils; phil++) {
    room->phil_state[phil] = THINKING;
    pthread_cond_init(&room->safe_to_eat[phil], NULL);
    room->phil_args[phil].phil_num = phil;
    room->phil_args[phil].num_cycles = num_cycles;
    room->phil_args[phil].room = room;
  }
}
