`neutu r --command --skeletonize <input> -o <output> --bodyid <bodyid> --force`

Conditions of updating a skeleton:

| Force Update | SWC Exist | Mutation ID | VSKL |
| :--- | :--- | :--- | :--- |
| X | N | X | X |
| N | Y | U | X |
| Y | Y | NA/U | X |
| Y | Y | M | G |

X: any; M: matched; N: unmatched; G: Greater than; NA: Not available

Examples of skeletonization:

Skeletonize a body:

<pre>
neutu --command --skeletonize --bodyid 5901284581 'http://localhost:1600?uuid=c315&segmentation=segmentation'
</pre>

