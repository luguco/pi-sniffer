/*
 * Generated by gdbus-codegen 2.56.4. DO NOT EDIT.
 *
 * The license of this code is the same as for the D-Bus interface description
 * it was derived from.
 */

#ifndef __SNIFFER_GENERATED_H__
#define __SNIFFER_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for com.signswift.sniffer */

#define PI_TYPE_SNIFFER (pi_sniffer_get_type ())
#define PI_SNIFFER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), PI_TYPE_SNIFFER, piSniffer))
#define PI_IS_SNIFFER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), PI_TYPE_SNIFFER))
#define PI_SNIFFER_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), PI_TYPE_SNIFFER, piSnifferIface))

struct _piSniffer;
typedef struct _piSniffer piSniffer;
typedef struct _piSnifferIface piSnifferIface;

struct _piSnifferIface
{
  GTypeInterface parent_iface;



  gboolean (*handle_settings) (
    piSniffer *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_greeting);

  gdouble  (*get_distance_limit) (piSniffer *object);

  gdouble  (*get_rssifactor) (piSniffer *object);

  gdouble  (*get_rssione_meter) (piSniffer *object);

  void (*notification) (
    piSniffer *object,
    gdouble arg_peopleClose,
    gdouble arg_peopleNearby);

};

GType pi_sniffer_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *pi_sniffer_interface_info (void);
guint pi_sniffer_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void pi_sniffer_complete_settings (
    piSniffer *object,
    GDBusMethodInvocation *invocation,
    gdouble peopleclose,
    gdouble peoplenearby);



/* D-Bus signal emissions functions: */
void pi_sniffer_emit_notification (
    piSniffer *object,
    gdouble arg_peopleClose,
    gdouble arg_peopleNearby);



/* D-Bus method calls: */
void pi_sniffer_call_settings (
    piSniffer *proxy,
    const gchar *arg_greeting,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean pi_sniffer_call_settings_finish (
    piSniffer *proxy,
    gdouble *out_peopleclose,
    gdouble *out_peoplenearby,
    GAsyncResult *res,
    GError **error);

gboolean pi_sniffer_call_settings_sync (
    piSniffer *proxy,
    const gchar *arg_greeting,
    gdouble *out_peopleclose,
    gdouble *out_peoplenearby,
    GCancellable *cancellable,
    GError **error);



/* D-Bus property accessors: */
gdouble pi_sniffer_get_rssifactor (piSniffer *object);
void pi_sniffer_set_rssifactor (piSniffer *object, gdouble value);

gdouble pi_sniffer_get_rssione_meter (piSniffer *object);
void pi_sniffer_set_rssione_meter (piSniffer *object, gdouble value);

gdouble pi_sniffer_get_distance_limit (piSniffer *object);
void pi_sniffer_set_distance_limit (piSniffer *object, gdouble value);


/* ---- */

#define PI_TYPE_SNIFFER_PROXY (pi_sniffer_proxy_get_type ())
#define PI_SNIFFER_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), PI_TYPE_SNIFFER_PROXY, piSnifferProxy))
#define PI_SNIFFER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), PI_TYPE_SNIFFER_PROXY, piSnifferProxyClass))
#define PI_SNIFFER_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PI_TYPE_SNIFFER_PROXY, piSnifferProxyClass))
#define PI_IS_SNIFFER_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), PI_TYPE_SNIFFER_PROXY))
#define PI_IS_SNIFFER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), PI_TYPE_SNIFFER_PROXY))

typedef struct _piSnifferProxy piSnifferProxy;
typedef struct _piSnifferProxyClass piSnifferProxyClass;
typedef struct _piSnifferProxyPrivate piSnifferProxyPrivate;

struct _piSnifferProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  piSnifferProxyPrivate *priv;
};

struct _piSnifferProxyClass
{
  GDBusProxyClass parent_class;
};

GType pi_sniffer_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (piSnifferProxy, g_object_unref)
#endif

void pi_sniffer_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
piSniffer *pi_sniffer_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
piSniffer *pi_sniffer_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void pi_sniffer_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
piSniffer *pi_sniffer_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
piSniffer *pi_sniffer_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define PI_TYPE_SNIFFER_SKELETON (pi_sniffer_skeleton_get_type ())
#define PI_SNIFFER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), PI_TYPE_SNIFFER_SKELETON, piSnifferSkeleton))
#define PI_SNIFFER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), PI_TYPE_SNIFFER_SKELETON, piSnifferSkeletonClass))
#define PI_SNIFFER_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PI_TYPE_SNIFFER_SKELETON, piSnifferSkeletonClass))
#define PI_IS_SNIFFER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), PI_TYPE_SNIFFER_SKELETON))
#define PI_IS_SNIFFER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), PI_TYPE_SNIFFER_SKELETON))

typedef struct _piSnifferSkeleton piSnifferSkeleton;
typedef struct _piSnifferSkeletonClass piSnifferSkeletonClass;
typedef struct _piSnifferSkeletonPrivate piSnifferSkeletonPrivate;

struct _piSnifferSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  piSnifferSkeletonPrivate *priv;
};

struct _piSnifferSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType pi_sniffer_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (piSnifferSkeleton, g_object_unref)
#endif

piSniffer *pi_sniffer_skeleton_new (void);


G_END_DECLS

#endif /* __SNIFFER_GENERATED_H__ */
