`neutu --command --general <config.json> <input> -o <output_url>`

Command JSON examples:

Sync bodies with a status of "roughly traced", "traced" or "leaves" with priority 3:
<pre>
{
  "command": "sync_skeleton",
  "priority": 3,
  "bodyStatus": ["roughly traced", "traced", "leaves"]
}
</pre>

Sync an explicit list of bodies, such as 1, 2, and 3:
<pre>
{
  "command": "sync_skeleton",
  "priority": 3,
  "bodyList": [1, 2, 3]
}
</pre>

Sync bodies in the file bodylist.txt, which is relative to the command config folder if the path is not absolute:
<pre>
{
  "command": "sync_skeleton",
  "priority": 3,
  "bodyList": "bodylist.txt"
}
</pre>

Sync bodies in the file bodlist.csv, which is relative to the command config folder if the path is not absolute (it has a head and the bodies are listed in column 0):
<pre>
{
  "command": "sync_skeleton",
  "priority": 3,
  "bodyList": {
    "source": "bodylist.csv",
    "column": 0,
    "hasHead": true
  }
}
</pre>