## FIFO queue

### Pro
It is very easy to implement the queue, and maintain the order in the queue structure. Queue has constantly O(1) add time. Hashtable also has O(1) add time, but it needs a good hash function. If the hash function is very good, the add time may much longer than queue.



### Cons
Searching time is much higher than hashtable. Hashtable has O(1), and queue has O(N) searching time, which means hashtable is much more efficient than queue.
