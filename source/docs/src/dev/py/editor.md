# Notes on create a code-editor


## Text Editor Windows

Max Api (p 530-531)


Max has a simple built-in text editor object that can display and edit text in conjunction with your object.

The routines described here let you create a text editor. When the editor window is about to be closed, your object could receive as many as three messages.

The first one, okclose, will be sent if the user has changed the text in the window. This is the standard okclose message that is sent to all "dirty" windows when they are about to be closed,  but the text editor window object passes it on to you instead of doing anything itself. Refer to the section on Window Messages for a description of how to write a method for the `okclose` message. It’s not required that you write one—if you don’t, the behavior of the window will be determined by the setting of the window’s `w_scratch` bit. If it’s set, no confirmation will be asked when a dirty window is closed (and no okclose message will be sent to the text editor either).

The second message, edclose, requires a method that should be added to your object at initialization time. 

The third message, edSave, allows you to gain access to the text before it is saved, or save it yourself.

## Forum Post

see also: https://cycling74.com/forums/text-editor-without-dirty-bit

```c
sprintf(msg, "Save changes to "%s" before closing?", filename->s_name);
// give our owner a chance to take control
okclose = zgetfn(p->v_owner, gensym("okclose"));
if (okclose) {
	short result = 0;	// changed from long to short to fix ppc bug
	object_method(p->v_owner, gensym("okclose"), msg, &result);
	switch (result) {
		case 0:		/* normal thing */
		case 1:		/* they changed the string */
			break;
		case 2:		/* don't put up a dialog, clear dirty bit */
			object_attr_setchar(p->v_owner, ps_dirty, 0);
		case 3:		/* don't put up a dialog */
			goto skipalert;
		case 4:		/* act as though user "cancelled" */
			return (void*) -1;
		default:
			break;
	}
}
res = wind_advise((t_object*) p, msg);
switch (res) {
	case ADVISE_SAVE:
		if (jwind_save(p))
			return (void*) -1;		// error saving, treat as cancel
		break;
	case ADVISE_CANCEL:
		return (void*) -1;
	case ADVISE_DISCARD:
		object_attr_setchar(p->v_owner, ps_dirty, 0);
		break;
}
```


## Showing a Text Editor

(maxapi p56-57)

Objects such as `coll` and `text` display a text editor window when you double-click. Users can edit the contents of the objects and save the updated data (or not). Here's how to do the same thing in your object.

First, if you want to support double-clicking on a non-UI object, you can respond to the dblclick message.

```c
class_addmethod(c, (method)myobject_dblclick, "dblclick", A_CANT, 0);

void myobject_dblclick(t_myobject *x)
{
    // open editor here
}
```
You'll need to add a `t_object` pointer to your object's data structure to hold the editor.

```c
typedef struct _myobject
{
	t_object m_obj;
	t_object *m_editor;
} t_myobject;
```

Initialize the `m_editor` field to NULL in your new instance routine. Then implement the `dblclick` method as follows:

```c
if (!x->m_editor)
	x->m_editor = object_new(CLASS_NOBOX, gensym("jed"), (t_object *)x, 0);
else
	object_attr_setchar(x->m_editor, gensym("visible"), 1);
```


The code above does the following: If the editor does not exist, we create one by making a `jed` object and passing our object as an argument. This permits the editor to tell our object when the window is closed. 

If the editor does exist, we set its visible attribute to 1, which brings the text editor window to the front.

To set the text of the edit window, we can send our `jed` object the `settext` message with a zero-terminated buffer of text.

We also provide a symbol specifying how the text is encoded. For best results, the text should be encoded as UTF-8.

Here is an example where we set a string to contain "Some text to edit" then pass it to the editor.

```c
char text[512];
strcpy(text,"Some text to edit");
object_method(x->m_editor, gensym("settext"), text, gensym("utf-8"));
```


The title attribute sets the window title of the text editor.

```c
object_attr_setsym(x->m_editor, gensym("title"), gensym("crazytext"));
```

When the user closes the text window, your object (or the object you passed as an argument when creating the editor) will be sent the `edclose` message.

```c
class_addmethod(c, (method)myobject_edclose, "edclose", A_CANT, 0);
```

The `edclose` method is responsible for doing something with the text. It should also zero the reference to the editor stored in the object, because it will be freed. A pointer to the text pointer is passed, along with its size. The encoding of the text is always UTF-8.

```c
void myobject_edclose(t_myobject *x, char **ht, long size)
{
    // do something with the text
	x->m_editor = NULL; 
}

```

If your object will be showing the contents of a text file, you are still responsible for setting the initial text, but you can assign a file so that the editor will save the text data when the user chooses Save from the File menu. To assign a file,
use the filename message, assuming you have a filename and path ID.

```c
object_method(x->m_editor, gensym("filename"), x->m_myfilename, x->m_mypath);
```

The filename message will set the title of the text editor window, but you can use the title attribute to override the simple filename. For example, you might want the name of your object to precede the filename:

```c
char titlename[512];
sprintf(titlename, "myobject: %s", x->m_myfilename);
object_attr_setsym(x->m_editor, gensym("title"), gensym(titlename));
```

Each time the user chooses Save, your object will receive an `edsave` message. If you return zero from your `edsave` method, the editor will proceed with saving the text in a file. If you return non-zero, the editor assumes you have taken care of saving the text. The general idea is that when the user wants to save the text, it is either updated inside your object, updated in a file, or both. As an example, the js object uses its `edsave` message to trigger a recompile of the Javascript code. But it also returns 0 from its `edsave` method so that the text editor will update the script file. Except for the return value, the prototype of the `edsave` method is identical to the `edclose` method.

```c
class_addmethod(c, (method)myobject_edsave, "edsave", A_CANT, 0); long myobject_edsave(t_myobject *x, char **ht, long size)
{
    // do something with the text
    return 0;       // tell editor it can save the text
}
```