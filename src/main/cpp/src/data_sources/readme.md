This directory holds all the data sources that Nova needs

All the data sources are implemented as singletons. This allows universal access while ensuring that they only have
one instance active, which is exactly what I want

Most of the data sources are a loader and a hash map. This is pretty close to what I want. I have functions for
retrieving data, but none for storing data (this will probably change). I don't want the main renderer to change the
code for a shader, for instance
