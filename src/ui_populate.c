#include "ui_internal.h"
#include <string.h>
#include <strings.h>   /* for strcasecmp */
#include <stdlib.h>

/* item classes */
Elm_Genlist_Item_Class itc_artist;
Elm_Genlist_Item_Class itc_album;
Elm_Genlist_Item_Class itc_album_header;
Elm_Genlist_Item_Class itc_track;

/* ------------------------------
   Generic text + del functions
   ------------------------------ */
static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
    Item_Data *id = data;
    if (!id) return NULL;

    switch (id->type) {
    case ITEM_ARTIST:
        /* artists use u.name */
        return strdup(id->u.name ? id->u.name : "");

    case ITEM_ALBUM_HEADER:
        /* album headers use u.album */
        return strdup(id->u.album && id->u.album->album
                      ? id->u.album->album
                      : "");

    case ITEM_TRACK:
        /* tracks use u.track */
        return strdup(id->u.track && id->u.track->title
                      ? id->u.track->title
                      : "");

    default:
        return NULL;
    }
}

static void
_gl_del(void *data, Evas_Object *obj)
{
    free(data);
}

/* ------------------------------
   Album item callbacks (for thumbnails)
   ------------------------------ */

static char *
_album_text_get(void *data, Evas_Object *obj, const char *part)
{
    Item_Data *id = data;
    Album_Entry *a = id->u.album;

    if (!strcmp(part, "elm.text"))
        return strdup(a && a->album ? a->album : "");

    return NULL;
}

static Evas_Object *
_album_content_get(void *data, Evas_Object *obj, const char *part)
{
    if (strcmp(part, "elm.swallow.icon"))
        return NULL;

    Item_Data *id = data;
    Album_Entry *a = id->u.album;

    Evas_Object *img = elm_image_add(obj);
    elm_image_aspect_fixed_set(img, EINA_TRUE);

    const char *path = (a && a->art_path) ? a->art_path : "data/noart.png";
    elm_image_file_set(img, path, NULL);

    evas_object_show(img);
    return img;
}

static void
_album_del(void *data, Evas_Object *obj)
{
    free(data);
}

/* ------------------------------
   Populate Artists
   ------------------------------ */
void
populate_artists(Player_State *ps)
{
    elm_genlist_clear(ps->genlist);

    char *name;
    Eina_List *l;

    EINA_LIST_FOREACH(ps->lib->artists, l, name) {
        Item_Data *id = calloc(1, sizeof(Item_Data));
        id->type = ITEM_ARTIST;
        id->u.name = name;

        elm_genlist_item_append(ps->genlist, &itc_artist, id, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, ps);
    }
}

/* ------------------------------
   Populate Albums (grouped by artist)
   ------------------------------ */
void
populate_albums(Player_State *ps)
{
    elm_genlist_clear(ps->genlist);

    Album_Entry *ae;
    Eina_List *l;
    const char *last_artist = NULL;

    EINA_LIST_FOREACH(ps->lib->albums, l, ae) {

        /* Insert artist header when artist changes */
        if (!last_artist || strcasecmp(last_artist, ae->artist) != 0) {
            Item_Data *id_header = calloc(1, sizeof(Item_Data));
            id_header->type = ITEM_ARTIST;
            id_header->u.name = ae->artist;

            elm_genlist_item_append(ps->genlist, &itc_artist, id_header, NULL,
                                    ELM_GENLIST_ITEM_NONE, NULL, ps);

            last_artist = ae->artist;
        }

        /* Insert album with thumbnail */
        Item_Data *id = calloc(1, sizeof(Item_Data));
        id->type = ITEM_ALBUM;
        id->u.album = ae;

        elm_genlist_item_append(ps->genlist, &itc_album, id, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, ps);
    }
}

/* ------------------------------
   Populate Tracks (grouped by album)
   ------------------------------ */
void
populate_tracks(Player_State *ps)
{
    elm_genlist_clear(ps->genlist);

    Album_Entry *ae;
    Eina_List *l;

    EINA_LIST_FOREACH(ps->lib->albums, l, ae) {

        /* Album header WITH cover: store Album_Entry* */
        Item_Data *id_header = calloc(1, sizeof(Item_Data));
        id_header->type = ITEM_ALBUM_HEADER;
        id_header->u.album = ae;

        elm_genlist_item_append(ps->genlist, &itc_album_header, id_header, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, ps);

        /* Tracks for this album */
        Eina_List *tracks = eina_hash_find(ps->lib->album_tracks, ae->album);
        Track *t;
        Eina_List *lt;

        EINA_LIST_FOREACH(tracks, lt, t) {
            Item_Data *id = calloc(1, sizeof(Item_Data));
            id->type = ITEM_TRACK;
            id->u.track = t;

            elm_genlist_item_append(ps->genlist, &itc_track, id, NULL,
                                    ELM_GENLIST_ITEM_NONE, NULL, ps);
        }
    }
}

/* ------------------------------
   Populate Albums Filtered by Artist
   ------------------------------ */
void
populate_albums_by_artist(Player_State *ps, const char *artist)
{
    elm_genlist_clear(ps->genlist);

    Album_Entry *ae;
    Eina_List *l;

    EINA_LIST_FOREACH(ps->lib->albums, l, ae) {
        if (strcasecmp(ae->artist, artist) == 0) {
            Item_Data *id = calloc(1, sizeof(Item_Data));
            id->type = ITEM_ALBUM;
            id->u.album = ae;

            elm_genlist_item_append(ps->genlist, &itc_album, id, NULL,
                                    ELM_GENLIST_ITEM_NONE, NULL, ps);
        }
    }
}

/* ------------------------------
   Init Item Classes
   ------------------------------ */
void
ui_populate_init(void)
{
    memset(&itc_artist, 0, sizeof(itc_artist));
    itc_artist.item_style = "default";
    itc_artist.func.text_get = _gl_text_get;
    itc_artist.func.del = _gl_del;

    memset(&itc_album, 0, sizeof(itc_album));
    itc_album.item_style = "thumb";   /* thumbnail style */
    itc_album.func.text_get = _album_text_get;
    itc_album.func.content_get = _album_content_get;
    itc_album.func.del = _album_del;

    memset(&itc_album_header, 0, sizeof(itc_album_header));
    itc_album_header.item_style = "default";
    itc_album_header.func.text_get = _gl_text_get;
    itc_album_header.func.content_get = _album_content_get;  /* cover in tracks view */
    itc_album_header.func.del = _gl_del;

    memset(&itc_track, 0, sizeof(itc_track));
    itc_track.item_style = "default";
    itc_track.func.text_get = _gl_text_get;
    itc_track.func.del = _gl_del;
}
