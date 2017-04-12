# CPU-Connected-Component-Labelling

This is a fast O(n) linear time CPU implementation of the famous connected component labelling problem. The algorithm implemented here
has a O(n) memory complexity; n being the number of elements in the input map.

For example, in problems where we have images like these -
![Alt text](http://members.cbio.mines-paristech.fr/~nvaroquaux/formations/scipy-lecture-notes/_images/plot_synthetic_data_1.png "Images with connected regions")

We desire to find the number of "blobs" or connected regions. The algorithm implemented here, takes the image and the label of the
connected region and spits out the number of such regions in the image.

Example usage in the main() method. Have fun!:D
