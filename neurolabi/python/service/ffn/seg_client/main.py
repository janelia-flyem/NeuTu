import os
import sys
import sqlite3 as sql
from functools import wraps
import time
import requests

DB_FILE='status.db'
SX,EX=0,5000
SY,EY=0,5000
SZ,EZ=1024,8000
BX,BY,BZ=256,256,256
URL='http://10.14.111.154:4321/em_seg'

def requireDB(func):
    @wraps(func)
    def inner(*args,**kwargs):
        with sql.connect(DB_FILE) as con:
            con.row_factory=sql.Row
            query=r"select count(*) from sqlite_master where type='table' and name='main'"
            if con.execute(query).fetchone()[0]==0:
                query=r'''create table main(id int primary key,
                                            x int, y int, z int,
                                            w int, h int, d int,
                                            status int, start_time text, end_time text,
                                            start_id int, end_id int);'''
                con.execute(query)
                id=0
                for z in range(SZ,EZ,BZ):
                    for y in range(SY,EY,BY):
                        for x in range(SX,EX,BX):
                            con.execute(r'''insert into main(id,x,y,z,w,h,d,status,start_time,
                                             end_time,start_id,end_id) values 
                                            (?,?,?,?,?,?,?,?,?,?,?,?) ''',
                                            (id,x,y,z,BX,BY,BZ,0,'','',0,0))
                            id+=1
            kwargs['cur']=con.cursor()
            return func(*args,**kwargs)
    return inner

@requireDB
def markDone(id,start_time,end_time,start_id,end_id,cur):
    cur.execute(r'''update main set status=1,start_time=?,end_time=?, 
            start_id=?, end_id=? where id=?''',(start_time,end_time,start_id,end_id,id))

@requireDB
def finished(cur):
    query=r"select count(*) from main where status=0"
    return cur.execute(query).fetchone()[0]==0

@requireDB
def nextTodo(cur):
    query=r"select * from main where status=0"
    row=cur.execute(query).fetchone()
    return row['id'],(row['x'],row['y'],row['z']),(row['w'],row['h'],row['d'])


def segmentAt(pos,size,start_id):
    rv=requests.post(URL,json={'offset':pos[::-1],'size':size[::-1],'start_id':start_id})
    if rv.status_code!=200:
        print('connect server failed...')
        return False,start_id
    if rv.json()['status']!=True:
        print('segment failed...')
        return False,start_id
    return True,int(rv.json()['next_id'])

@requireDB
def detail(id,cur):
    cur.execute('select * from main where id=?',(id,))
    rv=cur.fetchone()
    for k in rv.keys():
        print(k,'==>',rv[k])

@requireDB
def statistics(cur):
    total_blocks=cur.execute('select count(*) from main').fetchone()[0]
    finished_blocks=cur.execute('select count(*) from main where status=1').fetchone()[0]
    used_time=0
    for row in cur.execute('select * from main where status=1'):
        start=row['start_time']
        end=row['end_time']
        delta=time.mktime(time.strptime(end))-time.mktime(time.strptime(start))
        used_time+=delta
    percent=int(finished_blocks/total_blocks*100)
    print('total blocks:',total_blocks)
    print('segmented blocks:',finished_blocks,' ','|'*percent,'{0:.2f} %'.format(percent))
    m,s=divmod(used_time,60)
    h,m=divmod(m,60)
    print('time used:{0:0>2}:{1:0>2}:{2:0>2}'.format(*map(int,(h,m,s))))

@requireDB
def nextID(cur):
    next_id=-1
    query='select max(end_id) from main where status=1'
    rv=cur.execute(query).fetchone()
    if rv:
        next_id=rv[0] if rv[0] else -1
    return next_id+1

def main():
    while not finished():
        id,pos,size=nextTodo()
        start=time.ctime()
        start_id=nextID()
        status,next_id=segmentAt(pos,size,start_id)
        end=time.ctime()
        if status:
            print(id,start,'=>',end,pos,size,start_id,'=>',next_id-1,'done')
            markDone(id=id,start_time=start,end_time=end,start_id=start_id,end_id=next_id-1)
        else:
            print(start,end,id,pos,size,'failed')
    
if __name__=='__main__':
    if len(sys.argv)>1:
        if sys.argv[1]=='s':
            statistics()
        elif sys.argv[1]=='d':
            detail(sys.argv[2])
    else:
        main()
