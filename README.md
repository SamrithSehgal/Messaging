Sample Data:

NAME         AGE          ID          TIME
7 Samrith    17           1           10:02
4 Trev       17           2           10:04
 Time: 20:44
 WiFi Loc: 99 Shadywood
 
 Wifi Id: 3... Etc
----------------------------------------------------------------------------------------------------------------------------

I want you to apply the concept of storing concise records on pages to storing concise attributes in messages such that if there were attributes added or removed you could still store them concisely in the same message format

----------------------------------------------------------------------------------------------------------------------------

join key = attribute id
Fact table must maintain order because attr is found in table by going sequentially through attributes. So Attributes must be inserted in sequential order. IE. Attr1 -> Attr2 OK, Attr2 -> Attr1 BAD

-----------------------------------------------------------------------------------------------------------------------------

Page 1: Fact Page
{
  Attr1 Key: -1
  Attr2 Key: 3 (Page # of attribute)
  Attr3 Key: -1

  Num Pages: 1  
  num slots: 20
}

Page 3: Attr1 (Nonexistent) 
{
 NULL
}

Page 3: Attr2 
{
 WiFi Cons: 35
 Next page: 4
}

Page 4: Attr2
{
Wifi Cons: 2
Next page: 5
} etc...

Page 7: Attr 3 (Nonexistent)
{
 NULL
}







------------------------------------------------------------------------------------------------------------------------------

Ideas:


Use backend DB that just has all the attributes and then use that to recursivly create all facts(attributes) in fact tablex













































