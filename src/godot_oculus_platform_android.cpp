#include <godot_oculus_platform.h>
#include <godot_cpp/core/class_db.hpp>

static JavaVM *jvm;
static jobject jactivity;

using namespace godot;

GDOculusPlatform *GDOculusPlatform::singleton = nullptr;

void GDOculusPlatform::_bind_methods() {
	ClassDB::bind_method(D_METHOD("pump_messages"), &GDOculusPlatform::pump_messages);

	ClassDB::bind_method(D_METHOD("initialize_android", "app_id"), &GDOculusPlatform::initialize_android);
	ClassDB::bind_method(D_METHOD("initialize_android_async", "app_id"), &GDOculusPlatform::initialize_android_async);

	// USER
	ClassDB::bind_method(D_METHOD("user_get_logged_in_user_id"), &GDOculusPlatform::user_get_logged_in_user_id);
	ClassDB::bind_method(D_METHOD("user_get_logged_in_user_locale"), &GDOculusPlatform::user_get_logged_in_user_locale);
	ClassDB::bind_method(D_METHOD("user_get_is_viewer_entitled"), &GDOculusPlatform::user_get_is_viewer_entitled);
	ClassDB::bind_method(D_METHOD("user_get_logged_in_user"), &GDOculusPlatform::user_get_logged_in_user);
	ClassDB::bind_method(D_METHOD("user_get_user", "user_id"), &GDOculusPlatform::user_get_user);
	ClassDB::bind_method(D_METHOD("user_get_user_proof"), &GDOculusPlatform::user_get_user_proof);
	ClassDB::bind_method(D_METHOD("user_get_user_access_token"), &GDOculusPlatform::user_get_user_access_token);
	ClassDB::bind_method(D_METHOD("user_get_blocked_users"), &GDOculusPlatform::user_get_blocked_users);
	ClassDB::bind_method(D_METHOD("user_get_logged_in_user_friends"), &GDOculusPlatform::user_get_logged_in_user_friends);
	ClassDB::bind_method(D_METHOD("user_get_org_scoped_id", "user_id"), &GDOculusPlatform::user_get_org_scoped_id);
	ClassDB::bind_method(D_METHOD("user_get_sdk_accounts"), &GDOculusPlatform::user_get_sdk_accounts);
	ClassDB::bind_method(D_METHOD("user_launch_block_flow", "user_id"), &GDOculusPlatform::user_launch_block_flow);
	ClassDB::bind_method(D_METHOD("user_launch_unblock_flow", "user_id"), &GDOculusPlatform::user_launch_unblock_flow);
	ClassDB::bind_method(D_METHOD("user_launch_friend_request_flow", "user_id"), &GDOculusPlatform::user_launch_friend_request_flow);

	// ACHIEVEMENTS
	ClassDB::bind_method(D_METHOD("achievements_add_count", "achievement_name", "count"), &GDOculusPlatform::achievements_add_count);
	ClassDB::bind_method(D_METHOD("achievements_add_fields", "achievement_name", "fields"), &GDOculusPlatform::achievements_add_fields);
	ClassDB::bind_method(D_METHOD("achievements_unlock", "achievement_name"), &GDOculusPlatform::achievements_unlock);
	ClassDB::bind_method(D_METHOD("achievements_get_all_definitions"), &GDOculusPlatform::achievements_get_all_definitions);
	ClassDB::bind_method(D_METHOD("achievements_get_all_progress"), &GDOculusPlatform::achievements_get_all_progress);
	ClassDB::bind_method(D_METHOD("achievements_get_definitions_by_name", "achievement_names"), &GDOculusPlatform::achievements_get_definitions_by_name);
	ClassDB::bind_method(D_METHOD("achievements_get_progress_by_name", "achievement_names"), &GDOculusPlatform::achievements_get_progress_by_name);

	// IAP
	ClassDB::bind_method(D_METHOD("iap_get_viewer_purchases"), &GDOculusPlatform::iap_get_viewer_purchases);
	ClassDB::bind_method(D_METHOD("iap_get_viewer_purchases_durable_cache"), &GDOculusPlatform::iap_get_viewer_purchases_durable_cache);
	ClassDB::bind_method(D_METHOD("iap_get_products_by_sku", "sku_list"), &GDOculusPlatform::iap_get_products_by_sku);
	ClassDB::bind_method(D_METHOD("iap_consume_purchase", "sku"), &GDOculusPlatform::iap_consume_purchase);
	ClassDB::bind_method(D_METHOD("iap_launch_checkout_flow", "sku"), &GDOculusPlatform::iap_launch_checkout_flow);

	// ASSET FILE
	ClassDB::bind_method(D_METHOD("assetfile_get_list"), &GDOculusPlatform::assetfile_get_list);
	ClassDB::bind_method(D_METHOD("assetfile_status_by_id", "asset_id"), &GDOculusPlatform::assetfile_status_by_id);
	ClassDB::bind_method(D_METHOD("assetfile_status_by_name", "asset_name"), &GDOculusPlatform::assetfile_status_by_name);
	ClassDB::bind_method(D_METHOD("assetfile_download_by_id", "asset_id"), &GDOculusPlatform::assetfile_download_by_id);
	ClassDB::bind_method(D_METHOD("assetfile_download_by_name", "asset_name"), &GDOculusPlatform::assetfile_download_by_name);
	ClassDB::bind_method(D_METHOD("assetfile_download_cancel_by_id", "asset_id"), &GDOculusPlatform::assetfile_download_cancel_by_id);
	ClassDB::bind_method(D_METHOD("assetfile_download_cancel_by_name", "asset_name"), &GDOculusPlatform::assetfile_download_cancel_by_name);
	ClassDB::bind_method(D_METHOD("assetfile_delete_by_id", "asset_id"), &GDOculusPlatform::assetfile_delete_by_id);
	ClassDB::bind_method(D_METHOD("assetfile_delete_by_name", "asset_name"), &GDOculusPlatform::assetfile_delete_by_name);

	ADD_SIGNAL(MethodInfo("unhandled_message", PropertyInfo(Variant::DICTIONARY, "message")));
	ADD_SIGNAL(MethodInfo("assetfile_download_update", PropertyInfo(Variant::DICTIONARY, "download_info")));
	ADD_SIGNAL(MethodInfo("assetfile_download_finished", PropertyInfo(Variant::STRING, "asset_id")));
}

GDOculusPlatform *GDOculusPlatform::get_singleton() { return singleton; }

GDOculusPlatform::GDOculusPlatform() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

GDOculusPlatform::~GDOculusPlatform() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// INTERNAL METHODS
/////////////////////////////////////////////////

/// Retrieve existing promise from ID.
bool GDOculusPlatform::_get_promise(uint64_t p_promise_id, Ref<GDOculusPlatformPromise> &p_promise) {
	for (int i = 0; i < _promises.size(); i++) {
		Ref<GDOculusPlatformPromise> temp_promise = _promises.get(i);
		if (temp_promise->get_ids().has(p_promise_id)) {
			p_promise = temp_promise;
			_promises.remove_at(i);
			return true;
		}
	}

	ERR_FAIL_V_MSG(false, vformat("Unable to get promise with ID: %s", p_promise_id));
}

/// Rejects all promises in the rejection queue.
void GDOculusPlatform::_reject_promises() {
	for (int i = _promises_to_reject.size() - 1; i > -1; i--) {
		Ref<GDOculusPlatformPromise> temp_promise = _promises_to_reject.get(i);
		_promises_to_reject.remove_at(i);

		temp_promise->reject(temp_promise->saved_rejection_response);
	}
}

/// Returns a unique promise id. Only used for promises that should be rejected straight away.
uint64_t GDOculusPlatform::_get_reject_promise_id() {
	_last_promise_rejected_id += 1;
	return _last_promise_rejected_id;
}

/// Checks the OVR messages queue and handles them according to their type.
void GDOculusPlatform::pump_messages() {
	_reject_promises();

	ovrMessageHandle message = nullptr;

	// Process messages
	while ((message = ovr_PopMessage()) != nullptr) {
		switch (ovr_Message_GetType(message)) {
			case ovrMessage_PlatformInitializeAndroidAsynchronous:
				_process_initialize_android_async(message);
				break;

			case ovrMessage_Entitlement_GetIsViewerEntitled:
				_process_user_get_is_viewer_entitled(message);
				break;

			case ovrMessage_User_GetLoggedInUser:
				_process_user_get_logged_in_user(message);
				break;

			case ovrMessage_User_Get:
				_process_user_get_logged_in_user(message);
				break;

			case ovrMessage_User_GetUserProof:
				_process_user_get_user_proof(message);
				break;

			case ovrMessage_User_GetAccessToken:
				_process_user_get_user_access_token(message);
				break;

			case ovrMessage_User_GetBlockedUsers:
				_process_user_get_blocked_users(message);
				break;

			case ovrMessage_User_GetNextBlockedUserArrayPage:
				_process_user_get_blocked_users(message);
				break;

			case ovrMessage_User_GetLoggedInUserFriends:
				_process_user_get_logged_in_user_friends(message);
				break;

			case ovrMessage_User_GetNextUserArrayPage:
				_process_user_get_logged_in_user_friends(message);
				break;

			case ovrMessage_User_GetOrgScopedID:
				_process_user_get_org_scoped_id(message);
				break;

			case ovrMessage_User_GetSdkAccounts:
				_process_user_get_sdk_accounts(message);
				break;

			case ovrMessage_User_LaunchBlockFlow:
				_process_user_launch_block_flow(message);
				break;

			case ovrMessage_User_LaunchUnblockFlow:
				_process_user_launch_unblock_flow(message);
				break;

			case ovrMessage_User_LaunchFriendRequestFlow:
				_process_user_launch_friend_request_flow(message);
				break;

			case ovrMessage_Achievements_AddCount:
				_process_achievements_update(message);
				break;

			case ovrMessage_Achievements_AddFields:
				_process_achievements_update(message);
				break;

			case ovrMessage_Achievements_Unlock:
				_process_achievements_update(message);
				break;

			case ovrMessage_Achievements_GetAllDefinitions:
				_process_achievements_definitions(message);
				break;

			case ovrMessage_Achievements_GetDefinitionsByName:
				_process_achievements_definitions(message);
				break;

			case ovrMessage_Achievements_GetNextAchievementDefinitionArrayPage:
				_process_achievements_definitions(message);
				break;

			case ovrMessage_Achievements_GetAllProgress:
				_process_achievements_progress(message);
				break;

			case ovrMessage_Achievements_GetProgressByName:
				_process_achievements_progress(message);
				break;

			case ovrMessage_Achievements_GetNextAchievementProgressArrayPage:
				_process_achievements_progress(message);
				break;

			case ovrMessage_IAP_GetViewerPurchases:
				_process_iap_viewer_purchases(message);
				break;

			case ovrMessage_IAP_GetViewerPurchasesDurableCache:
				_process_iap_viewer_purchases(message);
				break;

			case ovrMessage_IAP_GetNextPurchaseArrayPage:
				_process_iap_viewer_purchases(message);
				break;

			case ovrMessage_IAP_GetProductsBySKU:
				_process_iap_products(message);
				break;

			case ovrMessage_IAP_GetNextProductArrayPage:
				_process_iap_products(message);
				break;

			case ovrMessage_IAP_ConsumePurchase:
				_process_iap_consume_purchase(message);
				break;

			case ovrMessage_IAP_LaunchCheckoutFlow:
				_process_iap_launch_checkout_flow(message);
				break;

			case ovrMessage_AssetFile_GetList:
				_process_assetfile_get_list(message);
				break;

			case ovrMessage_AssetFile_StatusById:
				_process_assetfile_get_status(message);

			case ovrMessage_AssetFile_StatusByName:
				_process_assetfile_get_status(message);

			case ovrMessage_AssetFile_DownloadById:
				_process_assetfile_download(message);
				break;

			case ovrMessage_AssetFile_DownloadByName:
				_process_assetfile_download(message);
				break;

			case ovrMessage_AssetFile_DownloadCancelById:
				_process_assetfile_download_cancel(message);
				break;

			case ovrMessage_AssetFile_DownloadCancelByName:
				_process_assetfile_download_cancel(message);
				break;

			case ovrMessage_AssetFile_DeleteById:
				_process_assetfile_delete(message);
				break;

			case ovrMessage_AssetFile_DeleteByName:
				_process_assetfile_delete(message);
				break;

			case ovrMessage_Notification_AssetFile_DownloadUpdate:
				_handle_download_update(message);
				break;

			default:
				_handle_unhandled_message(message);
		}

		ovr_FreeMessage(message);
	}
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// PLATFORM INITIALIZATION
/////////////////////////////////////////////////

/// Initialize Android Oculus Platform synchronously.
bool GDOculusPlatform::initialize_android(String p_app_id) {
	if (!ovr_IsPlatformInitialized()) {
		JNIEnv *gdjenv;
		_get_env(&gdjenv);

		if (ovr_PlatformInitializeAndroid(p_app_id.utf8().get_data(), jactivity, gdjenv) == ovrPlatformInitialize_Success) {
			return true;
		}

	} else {
		return true;
	}
}

/// Initialize Android Oculus Platform asynchronously.
/// @param p_app_id App ID
/// @return Promise to be resolved when the platform finishes initializing
Ref<GDOculusPlatformPromise> GDOculusPlatform::initialize_android_async(String p_app_id) {
	JNIEnv *gdjenv;
	_get_env(&gdjenv);

	ovrRequest req = ovr_PlatformInitializeAndroidAsynchronous(p_app_id.utf8().get_data(), jactivity, gdjenv);

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// USER
/////////////////////////////////////////////////

/// Requests the current user's id
/// @return The logged-in user's id as a String
String GDOculusPlatform::user_get_logged_in_user_id() {
	char native_id[21];
	ovrID u_id = ovr_GetLoggedInUserID();
	ovrID_ToString(native_id, sizeof(native_id), u_id);

	return String(native_id);
}

/// Requests the current user's locale
/// @return The logged-in user's locale as a String
String GDOculusPlatform::user_get_logged_in_user_locale() {
	const char *user_locale = ovr_GetLoggedInUserLocale();
	return String(user_locale);
}

/// Checks if the user is entitled to the current application.
/// @return Promise that will be fulfilled if the user is entitled to the app. It will be rejected (error) if the user is not entitled
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_is_viewer_entitled() {
	ovrRequest req = ovr_Entitlement_GetIsViewerEntitled();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests information about the current user.
/// @return Promise that will be fulfilled with the user's id, oculus_id, display_name, image_url, small_image_url and various Presence related information.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_logged_in_user() {
	ovrRequest req = ovr_User_GetLoggedInUser();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests information about an user from its ID.
/// @return Promise that will be fulfilled with the user's id, oculus_id, display_name, image_url, small_image_url and various Presence related information.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_user(String p_user_id) {
	ovrID u_id;
	if (ovrID_FromString(&u_id, p_user_id.utf8().get_data())) {
		ovrRequest req = ovr_User_Get(u_id);

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Invalid user id.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/// Requests a nonce used to verify the user.
/// @return Promise that will be fulfilled with the a nonce that can be used to verify the user.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_user_proof() {
	ovrRequest req = ovr_User_GetUserProof();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests the user access token suitable to make REST calls against graph.oculus.com.
/// @return Promise that will be contain a String token if fulfilled
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_user_access_token() {
	ovrRequest req = ovr_User_GetAccessToken();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests the user IDs of users blocked by the current user.
/// @return Promise that will contain user IDs as an Array of Strings if fulfilled
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_blocked_users() {
	ovrRequest req = ovr_User_GetBlockedUsers();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests the user IDs of the current user's friends.
/// @return Promise that will contain an Array of Dictionaries with information about each friend. Same format as the Dictionary returned by get_user()
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_logged_in_user_friends() {
	ovrRequest req = ovr_User_GetLoggedInUserFriends();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests the scoped org ID of a given user
/// @return Promise that will contain the org scoped ID of the given user as a String if fulfilled
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_org_scoped_id(String p_user_id) {
	ovrID u_id;
	if (ovrID_FromString(&u_id, p_user_id.utf8().get_data())) {
		ovrRequest req = ovr_User_GetOrgScopedID(u_id);

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Invalid user id.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/// Requests all the accounts belonging to this user.
/// @return Promise that will contain an Array of Dictionaries with the type of account and its ID, if fulfilled.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_get_sdk_accounts() {
	ovrRequest req = ovr_User_GetSdkAccounts();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests a block flow to block an user by its ID.
/// @return Promise that will contain a Dictionary with information if the user blocked or cancelled the request.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_launch_block_flow(String p_user_id) {
	ovrID u_id;
	if (ovrID_FromString(&u_id, p_user_id.utf8().get_data())) {
		ovrRequest req = ovr_User_LaunchBlockFlow(u_id);

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Invalid user id.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/// Requests an unblock flow to unblock an user by its ID.
/// @return Promise that will contain a Dictionary with information if the user unblocked or cancelled the request.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_launch_unblock_flow(String p_user_id) {
	ovrID u_id;
	if (ovrID_FromString(&u_id, p_user_id.utf8().get_data())) {
		ovrRequest req = ovr_User_LaunchUnblockFlow(u_id);

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Invalid user id.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/// Requests a friend request flow to send a friend request to a user with a given ID.
/// @return Promise that will contain a Dictionary with information if the user sent the friend request or cancelled.
Ref<GDOculusPlatformPromise> GDOculusPlatform::user_launch_friend_request_flow(String p_user_id) {
	ovrID u_id;
	if (ovrID_FromString(&u_id, p_user_id.utf8().get_data())) {
		ovrRequest req = ovr_User_LaunchFriendRequestFlow(u_id);

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Invalid user id.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// ACHIEVEMENTS
/////////////////////////////////////////////////

/// Requests an update for an achievement of type COUNT.
/// @return Promise that will contain a Dictionary with info about the result of the update request.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_add_count(String p_achievement_name, uint64_t p_count) {
	ovrRequest req = ovr_Achievements_AddCount(p_achievement_name.utf8().get_data(), p_count);

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests an update for an achievement of type BITFIELD.
/// @return Promise that will contain a Dictionary with info about the result of the update request.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_add_fields(String p_achievement_name, String p_fields) {
	ovrRequest req = ovr_Achievements_AddFields(p_achievement_name.utf8().get_data(), p_fields.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests an update for an achievement of type SIMPLE.
/// @return Promise that will contain a Dictionary with info about the result of the update request.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_unlock(String p_achievement_name) {
	ovrRequest req = ovr_Achievements_Unlock(p_achievement_name.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests all the achievement definitions.
/// @return Promise that will contain an Array of Dictionaries with info about each achievement.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_get_all_definitions() {
	ovrRequest req = ovr_Achievements_GetAllDefinitions();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests all the achievement definitions.
/// @return Promise that will contain an Array of Dictionaries with info about each achievement.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_get_all_progress() {
	ovrRequest req = ovr_Achievements_GetAllProgress();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests achievements definitions by name
/// @return Promise that will contain an Array of Dictionaries with info about each achievement.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_get_definitions_by_name(Array p_achievement_names) {
	int64_t achiev_arr_s = p_achievement_names.size();
	if (achiev_arr_s > 0 && achiev_arr_s <= INT_MAX) {
		const char **char_arr = new const char *[achiev_arr_s];
		for (size_t i = 0; i < p_achievement_names.size(); i++) {
			if (p_achievement_names[i].get_type() == Variant::STRING) {
				char_arr[i] = ((String)p_achievement_names[i]).utf8().get_data();

			} else {
				delete[] char_arr;
				Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
				String rejection_msg = "Invalid achievement name found in array. All achievement names must be Strings.";
				return_promise->saved_rejection_response = Array::make(rejection_msg);
				_promises_to_reject.push_back(return_promise);

				return return_promise;
			}
		}

		ovrRequest req = ovr_Achievements_GetDefinitionsByName(char_arr, achiev_arr_s);
		delete[] char_arr;

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else if (achiev_arr_s >= INT_MAX) {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Too many achievement names... How do you have more than 2147483647 achievements?";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Achievements array is empty.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/// Requests achievements progress by name
/// @return Promise that will contain an Array of Dictionaries with info about each achievement.
Ref<GDOculusPlatformPromise> GDOculusPlatform::achievements_get_progress_by_name(Array p_achievement_names) {
	int64_t achiev_arr_s = p_achievement_names.size();

	if (achiev_arr_s > 0 && achiev_arr_s <= INT_MAX) {
		const char **char_arr = new const char *[achiev_arr_s];
		for (size_t i = 0; i < p_achievement_names.size(); i++) {
			if (p_achievement_names[i].get_type() == Variant::STRING) {
				char_arr[i] = ((String)p_achievement_names[i]).utf8().get_data();

			} else {
				delete[] char_arr;
				Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
				String rejection_msg = "Invalid achievement name found in array. All achievement names must be Strings.";
				return_promise->saved_rejection_response = Array::make(rejection_msg);
				_promises_to_reject.push_back(return_promise);

				return return_promise;
			}
		}

		ovrRequest req = ovr_Achievements_GetProgressByName(char_arr, achiev_arr_s);
		delete[] char_arr;

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else if (achiev_arr_s >= INT_MAX) {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Too many achievement names... How do you have more than 2147483647 achievements?";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Achievements array is empty.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// IAP
/////////////////////////////////////////////////

/// Requests the current user's purchases. They include consumable items and durable items.
/// @return Promise that contains an Array of Dictionaries with information about each purchase.
Ref<GDOculusPlatformPromise> GDOculusPlatform::iap_get_viewer_purchases() {
	ovrRequest req = ovr_IAP_GetViewerPurchases();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests the current user's purchases. Includes only durable purchases.
/// @return Promise that contains an Array of Dictionaries with information about each purchase.
Ref<GDOculusPlatformPromise> GDOculusPlatform::iap_get_viewer_purchases_durable_cache() {
	ovrRequest req = ovr_IAP_GetViewerPurchasesDurableCache();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests a list of products by their SKU
/// @return Promise that contains an Array of Dictionaries with information about each product
Ref<GDOculusPlatformPromise> GDOculusPlatform::iap_get_products_by_sku(Array p_sku_list) {
	int64_t skus_arr_s = p_sku_list.size();

	if (skus_arr_s > 0 && skus_arr_s <= INT_MAX) {
		const char **char_arr = new const char *[skus_arr_s];
		for (size_t i = 0; i < p_sku_list.size(); i++) {
			if (p_sku_list[i].get_type() == Variant::STRING) {
				char_arr[i] = ((String)p_sku_list[i]).utf8().get_data();

			} else {
				delete[] char_arr;
				Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
				String rejection_msg = "Invalid SKU found in array. All SKUs must be Strings.";
				return_promise->saved_rejection_response = Array::make(rejection_msg);
				_promises_to_reject.push_back(return_promise);

				return return_promise;
			}
		}

		ovrRequest req = ovr_IAP_GetProductsBySKU(char_arr, skus_arr_s);
		delete[] char_arr;

		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
		_promises.push_back(return_promise);

		return return_promise;

	} else if (skus_arr_s >= INT_MAX) {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "Too many SKUs... How do you have more than 2147483647 SKUs?";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;

	} else {
		Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(_get_reject_promise_id()));
		String rejection_msg = "SKU array is empty.";
		return_promise->saved_rejection_response = Array::make(rejection_msg);
		_promises_to_reject.push_back(return_promise);

		return return_promise;
	}
}

/// Consumes a consumable item
/// @return Promise that contains true if the request was successful. It will error if unable to consume
Ref<GDOculusPlatformPromise> GDOculusPlatform::iap_consume_purchase(String p_sku) {
	ovrRequest req = ovr_IAP_ConsumePurchase(p_sku.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Launches the checkout flow
/// @return Promise that contains a Dictionary with information about the product. purchase_str_id will be empty if the user did not complete the purchase
Ref<GDOculusPlatformPromise> GDOculusPlatform::iap_launch_checkout_flow(String p_sku) {
	ovrRequest req = ovr_IAP_LaunchCheckoutFlow(p_sku.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// ASSET FILE
/////////////////////////////////////////////////

/// Requests a list of asset files associated with the app.
/// @return Promise that contains an Array of Dictionaries with information about each assetfile. Language packs have extra information.
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_get_list() {
	ovrRequest req = ovr_AssetFile_GetList();

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests information about a single asset file by ID.
/// @return Promise that contains a Dictionary with information about the assetfile. Language packs have extra information.
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_status_by_id(String p_asset_id) {
	ovrID n_asset_id;
	ovrID_FromString(&n_asset_id, p_asset_id.utf8().get_data());
	ovrRequest req = ovr_AssetFile_StatusById(n_asset_id);

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests information about a single asset file by name.
/// @return Promise that contains a Dictionary with information about the assetfile. Language packs have extra information.
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_status_by_name(String p_asset_name) {
	ovrRequest req = ovr_AssetFile_StatusByName(p_asset_name.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests to download an asset file by ID.
/// @return Promise that contains the result of the request as a Dictionary.
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_download_by_id(String p_asset_id) {
	ovrID n_asset_id;
	ovrID_FromString(&n_asset_id, p_asset_id.utf8().get_data());
	ovrRequest req = ovr_AssetFile_DownloadById(n_asset_id);

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests to download an asset file by name.
/// @return Promise that contains the result of the request as a Dictionary.
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_download_by_name(String p_asset_name) {
	ovrRequest req = ovr_AssetFile_DownloadByName(p_asset_name.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests to cancel a download of an assetfile by ID.
/// @return Promise that contains the result of the request as a Dictionary. The dictionary includes a "success" key to indicate if the request was successful
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_download_cancel_by_id(String p_asset_id) {
	ovrID n_asset_id;
	ovrID_FromString(&n_asset_id, p_asset_id.utf8().get_data());
	ovrRequest req = ovr_AssetFile_DownloadCancelById(n_asset_id);

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests to cancel a download of an assetfile by name.
/// @return Promise that contains the result of the request as a Dictionary. The dictionary includes a "success" key to indicate if the request was successful
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_download_cancel_by_name(String p_asset_name) {
	ovrRequest req = ovr_AssetFile_DownloadCancelByName(p_asset_name.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests to delete an assetfile by ID.
/// @return Promise that contains the result of the request as a Dictionary. The dictionary includes a "success" key to indicate if the request was successful
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_delete_by_id(String p_asset_id) {
	ovrID n_asset_id;
	ovrID_FromString(&n_asset_id, p_asset_id.utf8().get_data());
	ovrRequest req = ovr_AssetFile_DeleteById(n_asset_id);

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/// Requests to delete an assetfile by name.
/// @return Promise that contains the result of the request as a Dictionary. The dictionary includes a "success" key to indicate if the request was successful
Ref<GDOculusPlatformPromise> GDOculusPlatform::assetfile_delete_by_name(String p_asset_name) {
	ovrRequest req = ovr_AssetFile_DeleteByName(p_asset_name.utf8().get_data());

	Ref<GDOculusPlatformPromise> return_promise = memnew(GDOculusPlatformPromise(req));
	_promises.push_back(return_promise);

	return return_promise;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// INTERNAL PROCESSING METHODS
/////////////////////////////////////////////////

///// USERS
/////////////////////////////////////////////////

/// Processes android asynchronous initialization
void GDOculusPlatform::_process_initialize_android_async(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrPlatformInitializeHandle platform_init_resp = ovr_Message_GetPlatformInitialize(p_message);
		ovrPlatformInitializeResult platform_init_result = ovr_PlatformInitialize_GetResult(platform_init_resp);

		if (platform_init_result == ovrPlatformInitialize_Success) {
			if (_get_promise(msg_id, promise)) {
				promise->fulfill(Array::make(true));
			}

		} else {
			String gd_message = ovrPlatformInitializeResult_ToString(platform_init_result);

			if (_get_promise(msg_id, promise)) {
				promise->reject(Array::make(gd_message));
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes entitlement check
void GDOculusPlatform::_process_user_get_is_viewer_entitled(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(true));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the user information and transforms it into a dictionary
void GDOculusPlatform::_process_user_get_logged_in_user(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrUserHandle user_info_handle = ovr_Message_GetUser(p_message);
		Dictionary user_info_resp = _get_user_information(user_info_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(user_info_resp));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from the user's nonce request
void GDOculusPlatform::_process_user_get_user_proof(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrUserProofHandle user_proof = ovr_Message_GetUserProof(p_message);
		String user_nonce = ovr_UserProof_GetNonce(user_proof);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(user_nonce));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes an access token of the current user.
void GDOculusPlatform::_process_user_get_user_access_token(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		String access_token = ovr_Message_GetString(p_message);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(access_token));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes blocked user ids from current user. Paginated
void GDOculusPlatform::_process_user_get_blocked_users(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrBlockedUserArrayHandle blocked_users_handle = ovr_Message_GetBlockedUserArray(p_message);
		size_t blocked_users_array_size = ovr_BlockedUserArray_GetSize(blocked_users_handle);

		if (_get_promise(msg_id, promise)) {
			if (promise->get_ids().size() == 1 && promise->saved_fulfill_response.is_empty()) { // Only the first time
				promise->saved_fulfill_response = Array::make(Array());
			}

			for (size_t i = 0; i < blocked_users_array_size; i++) {
				ovrBlockedUserHandle blocked_user_handle = ovr_BlockedUserArray_GetElement(blocked_users_handle, i);

				char native_id[21];
				ovrID blocked_user_id = ovr_BlockedUser_GetId(blocked_user_handle);
				ovrID_ToString(native_id, sizeof(native_id), blocked_user_id);
				((Array)promise->saved_fulfill_response[0]).push_back(String(native_id));
			}

			if (!ovr_BlockedUserArray_HasNextPage(blocked_users_handle)) {
				promise->fulfill(promise->saved_fulfill_response);

			} else {
				ovrRequest new_req = ovr_User_GetNextBlockedUserArrayPage(blocked_users_handle);
				promise->add_id(new_req);
				_promises.push_back(promise);
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes user information about the current user's friends. Paginated
void GDOculusPlatform::_process_user_get_logged_in_user_friends(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrUserArrayHandle user_friends_handle = ovr_Message_GetUserArray(p_message);
		size_t user_friends_array_size = ovr_UserArray_GetSize(user_friends_handle);

		if (_get_promise(msg_id, promise)) {
			if (promise->get_ids().size() == 1 && promise->saved_fulfill_response.is_empty()) { // Only the first time
				promise->saved_fulfill_response = Array::make(Array());
			}

			for (size_t i = 0; i < user_friends_array_size; i++) {
				ovrUserHandle user_handle = ovr_UserArray_GetElement(user_friends_handle, i);
				Dictionary user_info = _get_user_information(user_handle);

				((Array)promise->saved_fulfill_response[0]).push_back(user_info);
			}

			if (!ovr_UserArray_HasNextPage(user_friends_handle)) {
				promise->fulfill(promise->saved_fulfill_response);

			} else {
				ovrRequest new_req = ovr_User_GetNextUserArrayPage(user_friends_handle);
				promise->add_id(new_req);
				_promises.push_back(promise);
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the org scoped id of a given user
void GDOculusPlatform::_process_user_get_org_scoped_id(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrOrgScopedIDHandle org_scoped_id_handle = ovr_Message_GetOrgScopedID(p_message);

		char native_id[21];
		ovrID org_scoped_id = ovr_OrgScopedID_GetID(org_scoped_id_handle);
		ovrID_ToString(native_id, sizeof(native_id), org_scoped_id);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(String(native_id)));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes sdk accounts (accounts in the headset) of the current user
void GDOculusPlatform::_process_user_get_sdk_accounts(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrSdkAccountArrayHandle accounts_array_handle = ovr_Message_GetSdkAccountArray(p_message);
		size_t accounts_size = ovr_SdkAccountArray_GetSize(accounts_array_handle);
		Array response_arr = Array();

		for (size_t i = 0; i < accounts_size; i++) {
			ovrSdkAccountHandle sdk_account_handle = ovr_SdkAccountArray_GetElement(accounts_array_handle, i);
			ovrSdkAccountType sdk_account_type = ovr_SdkAccount_GetAccountType(sdk_account_handle);
			ovrID sdk_user_id = ovr_SdkAccount_GetUserId(sdk_account_handle);

			String gd_sdk_account_type = ovrSdkAccountType_ToString(sdk_account_type);

			char native_id[21];
			ovrID_ToString(native_id, sizeof(native_id), sdk_user_id);

			Dictionary sdk_account_info;
			sdk_account_info["account_type"] = gd_sdk_account_type;
			sdk_account_info["account_id"] = gd_sdk_account_type;

			response_arr.push_back(sdk_account_info);
		}

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(response_arr));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the result of the block flow. Returns a Dictionary.
void GDOculusPlatform::_process_user_launch_block_flow(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrLaunchBlockFlowResultHandle block_flow_result_handle = ovr_Message_GetLaunchBlockFlowResult(p_message);

		Dictionary block_flow_result;
		block_flow_result["did_block"] = ovr_LaunchBlockFlowResult_GetDidBlock(block_flow_result_handle);
		block_flow_result["did_cancel"] = ovr_LaunchBlockFlowResult_GetDidCancel(block_flow_result_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(block_flow_result));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the result of the unblock flow. Returns a Dictionary.
void GDOculusPlatform::_process_user_launch_unblock_flow(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrLaunchUnblockFlowResultHandle unblock_flow_result_handle = ovr_Message_GetLaunchUnblockFlowResult(p_message);

		Dictionary unblock_flow_result;
		unblock_flow_result["did_unblock"] = ovr_LaunchUnblockFlowResult_GetDidUnblock(unblock_flow_result_handle);
		unblock_flow_result["did_cancel"] = ovr_LaunchUnblockFlowResult_GetDidCancel(unblock_flow_result_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(unblock_flow_result));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the result of the friend request flow. Returns a Dictionary.
void GDOculusPlatform::_process_user_launch_friend_request_flow(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrLaunchFriendRequestFlowResultHandle friend_req_flow_result_handle = ovr_Message_GetLaunchFriendRequestFlowResult(p_message);

		Dictionary friend_req_flow_result;
		friend_req_flow_result["did_send_request"] = ovr_LaunchFriendRequestFlowResult_GetDidSendRequest(friend_req_flow_result_handle);
		friend_req_flow_result["did_cancel"] = ovr_LaunchFriendRequestFlowResult_GetDidCancel(friend_req_flow_result_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(friend_req_flow_result));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

///// ACHIEVEMENTS
/////////////////////////////////////////////////

/// Processes the response from a request to update an achievement. Returns a Dictionary if fulfilled
void GDOculusPlatform::_process_achievements_update(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAchievementUpdateHandle achievents_update_handle = ovr_Message_GetAchievementUpdate(p_message);
		Dictionary achievements_update_resp;

		achievements_update_resp["name"] = String(ovr_AchievementUpdate_GetName(achievents_update_handle));
		achievements_update_resp["just_unlocked"] = ovr_AchievementUpdate_GetJustUnlocked(achievents_update_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(achievements_update_resp));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request of achievements_get_all_definitions or achievements_get_definitions_by_name
void GDOculusPlatform::_process_achievements_definitions(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAchievementDefinitionArrayHandle achiev_defs_handle = ovr_Message_GetAchievementDefinitionArray(p_message);
		size_t achiev_defs_array_size = ovr_AchievementDefinitionArray_GetSize(achiev_defs_handle);

		if (_get_promise(msg_id, promise)) {
			if (promise->get_ids().size() == 1 && promise->saved_fulfill_response.is_empty()) { // Only the first time
				promise->saved_fulfill_response = Array::make(Array());
			}

			for (size_t i = 0; i < achiev_defs_array_size; i++) {
				ovrAchievementDefinitionHandle achiev_def_handle = ovr_AchievementDefinitionArray_GetElement(achiev_defs_handle, i);
				Dictionary achiev_def;

				achiev_def["name"] = String(ovr_AchievementDefinition_GetName(achiev_def_handle));
				achiev_def["bitfield_length"] = ovr_AchievementDefinition_GetBitfieldLength(achiev_def_handle);
				achiev_def["target"] = (uint64_t)ovr_AchievementDefinition_GetTarget(achiev_def_handle);

				ovrAchievementType achiev_type = ovr_AchievementDefinition_GetType(achiev_def_handle);
				achiev_def["type"] = String(ovrAchievementType_ToString(achiev_type));

				((Array)promise->saved_fulfill_response[0]).push_back(achiev_def);
			}

			if (!ovr_AchievementDefinitionArray_HasNextPage(achiev_defs_handle)) {
				promise->fulfill(promise->saved_fulfill_response);

			} else {
				ovrRequest new_req = ovr_Achievements_GetNextAchievementDefinitionArrayPage(achiev_defs_handle);
				promise->add_id(new_req);
				_promises.push_back(promise);
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request of achievements_get_all_progress or achievements_get_progress_by_name
void GDOculusPlatform::_process_achievements_progress(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAchievementProgressArrayHandle achievs_prog_handle = ovr_Message_GetAchievementProgressArray(p_message);
		size_t achievs_prog_array_size = ovr_AchievementProgressArray_GetSize(achievs_prog_handle);

		if (_get_promise(msg_id, promise)) {
			if (promise->get_ids().size() == 1 && promise->saved_fulfill_response.is_empty()) { // Only the first time
				promise->saved_fulfill_response = Array::make(Array());
			}

			for (size_t i = 0; i < achievs_prog_array_size; i++) {
				ovrAchievementProgressHandle achiev_prog_handle = ovr_AchievementProgressArray_GetElement(achievs_prog_handle, i);
				Dictionary achiev_prog;

				achiev_prog["name"] = String(ovr_AchievementProgress_GetName(achiev_prog_handle));
				achiev_prog["current_count"] = (uint64_t)ovr_AchievementProgress_GetCount(achiev_prog_handle);
				achiev_prog["current_bitfield"] = String(ovr_AchievementProgress_GetBitfield(achiev_prog_handle));
				achiev_prog["is_unlocked"] = ovr_AchievementProgress_GetIsUnlocked(achiev_prog_handle);
				achiev_prog["unlock_time"] = (uint64_t)ovr_AchievementProgress_GetUnlockTime(achiev_prog_handle);

				((Array)promise->saved_fulfill_response[0]).push_back(achiev_prog);
			}

			if (!ovr_AchievementProgressArray_HasNextPage(achievs_prog_handle)) {
				promise->fulfill(promise->saved_fulfill_response);

			} else {
				ovrRequest new_req = ovr_Achievements_GetNextAchievementProgressArrayPage(achievs_prog_handle);
				promise->add_id(new_req);
				_promises.push_back(promise);
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

///// IAP
/////////////////////////////////////////////////

/// Processes the response from a request to get the viewer purchases. Used for both durable cache only and all purchases
void GDOculusPlatform::_process_iap_viewer_purchases(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrPurchaseArrayHandle v_purchases_arr_handle = ovr_Message_GetPurchaseArray(p_message);
		size_t v_purchases_arr_s = ovr_PurchaseArray_GetSize(v_purchases_arr_handle);

		if (_get_promise(msg_id, promise)) {
			if (promise->get_ids().size() == 1 && promise->saved_fulfill_response.is_empty()) { // Only the first time
				promise->saved_fulfill_response = Array::make(Array());
			}

			for (size_t i = 0; i < v_purchases_arr_s; i++) {
				ovrPurchaseHandle v_purchase_handle = ovr_PurchaseArray_GetElement(v_purchases_arr_handle, i);
				Dictionary v_purchase;

				v_purchase["sku"] = String(ovr_Purchase_GetSKU(v_purchase_handle));
				v_purchase["reporting_id"] = String(ovr_Purchase_GetReportingId(v_purchase_handle));
				v_purchase["purchase_str_id"] = String(ovr_Purchase_GetPurchaseStrID(v_purchase_handle));
				v_purchase["grant_time"] = (uint64_t)ovr_Purchase_GetGrantTime(v_purchase_handle);
				v_purchase["expiration_time"] = (uint64_t)ovr_Purchase_GetExpirationTime(v_purchase_handle);
				v_purchase["developer_payload"] = String(ovr_Purchase_GetDeveloperPayload(v_purchase_handle));

				((Array)promise->saved_fulfill_response[0]).push_back(v_purchase);
			}

			if (!ovr_PurchaseArray_HasNextPage(v_purchases_arr_handle)) {
				promise->fulfill(promise->saved_fulfill_response);

			} else {
				ovrRequest new_req = ovr_IAP_GetNextPurchaseArrayPage(v_purchases_arr_handle);
				promise->add_id(new_req);
				_promises.push_back(promise);
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request to get products by SKU
void GDOculusPlatform::_process_iap_products(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrProductArrayHandle products_arr_handle = ovr_Message_GetProductArray(p_message);
		size_t products_arr_s = ovr_ProductArray_GetSize(products_arr_handle);

		if (_get_promise(msg_id, promise)) {
			if (promise->get_ids().size() == 1 && promise->saved_fulfill_response.is_empty()) { // Only the first time
				promise->saved_fulfill_response = Array::make(Array());
			}

			for (size_t i = 0; i < products_arr_s; i++) {
				ovrProductHandle product_handle = ovr_ProductArray_GetElement(products_arr_handle, i);
				Dictionary product;

				product["sku"] = String(ovr_Product_GetSKU(product_handle));
				product["name"] = String(ovr_Product_GetName(product_handle));
				product["formatted_price"] = String(ovr_Product_GetFormattedPrice(product_handle));
				product["description"] = String(ovr_Product_GetDescription(product_handle));

				((Array)promise->saved_fulfill_response[0]).push_back(product);
			}

			if (!ovr_ProductArray_HasNextPage(products_arr_handle)) {
				promise->fulfill(promise->saved_fulfill_response);

			} else {
				ovrRequest new_req = ovr_IAP_GetNextProductArrayPage(products_arr_handle);
				promise->add_id(new_req);
				_promises.push_back(promise);
			}
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the result of a request to consume a consumable item
void GDOculusPlatform::_process_iap_consume_purchase(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(true));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the result of the block flow. Returns a Dictionary with purchase_str_id empty if the user did not complete the purchase.
void GDOculusPlatform::_process_iap_launch_checkout_flow(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrPurchaseHandle purchase_handle = ovr_Message_GetPurchase(p_message);

		Dictionary purchase;

		purchase["sku"] = String(ovr_Purchase_GetSKU(purchase_handle));
		purchase["reporting_id"] = String(ovr_Purchase_GetReportingId(purchase_handle));
		purchase["purchase_str_id"] = String(ovr_Purchase_GetPurchaseStrID(purchase_handle));
		purchase["grant_time"] = (uint64_t)ovr_Purchase_GetGrantTime(purchase_handle);
		purchase["expiration_time"] = (uint64_t)ovr_Purchase_GetExpirationTime(purchase_handle);
		purchase["developer_payload"] = String(ovr_Purchase_GetDeveloperPayload(purchase_handle));

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(purchase));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

///// ASSET FILE
/////////////////////////////////////////////////

/// Processes the response from a request to get a list of asset files
void GDOculusPlatform::_process_assetfile_get_list(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAssetDetailsArrayHandle assetfile_arr_handle = ovr_Message_GetAssetDetailsArray(p_message);
		size_t assetfile_arr_size = ovr_AssetDetailsArray_GetSize(assetfile_arr_handle);

		Array resp_arr = Array();

		for (size_t i = 0; i < assetfile_arr_size; i++) {
			ovrAssetDetailsHandle assetfile_handle = ovr_AssetDetailsArray_GetElement(assetfile_arr_handle, i);
			Dictionary assetfile;

			ovrID assetfile_id = ovr_AssetDetails_GetAssetId(assetfile_handle);
			char native_id[21];
			ovrID_ToString(native_id, sizeof(native_id), assetfile_id);

			String assetfile_type = ovr_AssetDetails_GetAssetType(assetfile_handle);

			assetfile["id"] = String(native_id);
			assetfile["type"] = assetfile_type;
			assetfile["download_status"] = String(ovr_AssetDetails_GetDownloadStatus(assetfile_handle));
			assetfile["file_path"] = String(ovr_AssetDetails_GetFilepath(assetfile_handle));
			assetfile["iap_status"] = String(ovr_AssetDetails_GetIapStatus(assetfile_handle));
			assetfile["metadata"] = String(ovr_AssetDetails_GetMetadata(assetfile_handle));

			if (assetfile_type == "language_pack") {
				ovrLanguagePackInfoHandle language_handle = ovr_AssetDetails_GetLanguage(assetfile_handle);
				Dictionary language_info;

				language_info["english_name"] = String(ovr_LanguagePackInfo_GetEnglishName(language_handle));
				language_info["native_name"] = String(ovr_LanguagePackInfo_GetNativeName(language_handle));
				language_info["tag"] = String(ovr_LanguagePackInfo_GetTag(language_handle)); // BCP47 format

				assetfile["language_info"] = language_info;
			}

			resp_arr.push_back(assetfile);
		}

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(resp_arr));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request to get the status of a single asset file
void GDOculusPlatform::_process_assetfile_get_status(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAssetDetailsHandle assetfile_handle = ovr_Message_GetAssetDetails(p_message);
		Dictionary assetfile;

		ovrID assetfile_id = ovr_AssetDetails_GetAssetId(assetfile_handle);
		char native_id[21];
		ovrID_ToString(native_id, sizeof(native_id), assetfile_id);

		String assetfile_type = ovr_AssetDetails_GetAssetType(assetfile_handle);

		assetfile["id"] = String(native_id);
		assetfile["type"] = assetfile_type;
		assetfile["download_status"] = String(ovr_AssetDetails_GetDownloadStatus(assetfile_handle));
		assetfile["file_path"] = String(ovr_AssetDetails_GetFilepath(assetfile_handle));
		assetfile["iap_status"] = String(ovr_AssetDetails_GetIapStatus(assetfile_handle));
		assetfile["metadata"] = String(ovr_AssetDetails_GetMetadata(assetfile_handle));

		if (assetfile_type == "language_pack") {
			ovrLanguagePackInfoHandle language_handle = ovr_AssetDetails_GetLanguage(assetfile_handle);
			Dictionary language_info;

			language_info["english_name"] = String(ovr_LanguagePackInfo_GetEnglishName(language_handle));
			language_info["native_name"] = String(ovr_LanguagePackInfo_GetNativeName(language_handle));
			language_info["tag"] = String(ovr_LanguagePackInfo_GetTag(language_handle)); // BCP47 format

			assetfile["language_info"] = language_info;
		}

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(assetfile));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request to download a single asset file
void GDOculusPlatform::_process_assetfile_download(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAssetFileDownloadResultHandle assetfile_download_handle = ovr_Message_GetAssetFileDownloadResult(p_message);
		Dictionary assetfile_download_resp;

		ovrID assetfile_id = ovr_AssetFileDownloadResult_GetAssetId(assetfile_download_handle);
		char native_id[21];
		ovrID_ToString(native_id, sizeof(native_id), assetfile_id);

		assetfile_download_resp["id"] = String(native_id);
		assetfile_download_resp["file_path"] = String(ovr_AssetFileDownloadResult_GetFilepath(assetfile_download_handle));

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(assetfile_download_resp));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request to cancel a download of a single asset file
void GDOculusPlatform::_process_assetfile_download_cancel(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAssetFileDownloadCancelResultHandle assetfile_download_c_handle = ovr_Message_GetAssetFileDownloadCancelResult(p_message);
		Dictionary assetfile_download_c_resp;

		ovrID assetfile_id = ovr_AssetFileDownloadCancelResult_GetAssetId(assetfile_download_c_handle);
		char native_id[21];
		ovrID_ToString(native_id, sizeof(native_id), assetfile_id);

		assetfile_download_c_resp["id"] = String(native_id);
		assetfile_download_c_resp["file_path"] = String(ovr_AssetFileDownloadCancelResult_GetFilepath(assetfile_download_c_handle));
		assetfile_download_c_resp["success"] = ovr_AssetFileDownloadCancelResult_GetSuccess(assetfile_download_c_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(assetfile_download_c_resp));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

/// Processes the response from a request to delete a single asset file
void GDOculusPlatform::_process_assetfile_delete(ovrMessageHandle p_message) {
	ovrRequest msg_id = ovr_Message_GetRequestID(p_message);
	Ref<GDOculusPlatformPromise> promise;

	if (!ovr_Message_IsError(p_message)) {
		ovrAssetFileDeleteResultHandle assetfile_delete_handle = ovr_Message_GetAssetFileDeleteResult(p_message);
		Dictionary assetfile_delete_resp;

		ovrID assetfile_id = ovr_AssetFileDeleteResult_GetAssetId(assetfile_delete_handle);
		char native_id[21];
		ovrID_ToString(native_id, sizeof(native_id), assetfile_id);

		assetfile_delete_resp["id"] = String(native_id);
		assetfile_delete_resp["file_path"] = String(ovr_AssetFileDeleteResult_GetFilepath(assetfile_delete_handle));
		assetfile_delete_resp["success"] = ovr_AssetFileDeleteResult_GetSuccess(assetfile_delete_handle);

		if (_get_promise(msg_id, promise)) {
			promise->fulfill(Array::make(assetfile_delete_resp));
		}

	} else {
		_handle_default_process_error(p_message, msg_id, promise);
	}
}

///// PROCESSING HELPERS
/////////////////////////////////////////////////

/// Handles unhandled messages
void GDOculusPlatform::_handle_unhandled_message(ovrMessageHandle message) {
	String gd_message = ovr_Message_GetString(message);

	ovrMessageType msg_type = ovr_Message_GetType(message);
	String gd_msg_type = ovrMessageType_ToString(msg_type);

	Dictionary gd_msg;
	gd_msg["type"] = gd_msg_type;
	gd_msg["message"] = gd_message;

	emit_signal("unhandled_message", gd_msg);
}

/// Helper function to handle most common errors when processing.
void GDOculusPlatform::_handle_default_process_error(ovrMessageHandle p_message, ovrRequest p_msg_id, Ref<GDOculusPlatformPromise> &p_promise) {
	ovrErrorHandle ovr_err = ovr_Message_GetError(p_message);
	String gd_message = ovr_Error_GetMessage(ovr_err);

	if (_get_promise(p_msg_id, p_promise)) {
		p_promise->reject(Array::make(gd_message));
	}
}

// Helper function to get a Dictionary with information about a single user.
Dictionary GDOculusPlatform::_get_user_information(ovrUserHandle p_user_handle) {
	Dictionary user_info_resp;
	Dictionary user_info_presence;

	user_info_resp["display_name"] = ovr_User_GetDisplayName(p_user_handle);

	char native_id[21];
	ovrID u_id = ovr_User_GetID(p_user_handle);
	ovrID_ToString(native_id, sizeof(native_id), u_id);
	user_info_resp["id"] = String(native_id);

	user_info_resp["oculus_id"] = ovr_User_GetOculusID(p_user_handle);
	user_info_resp["image_url"] = ovr_User_GetImageUrl(p_user_handle);
	user_info_resp["small_image_url"] = ovr_User_GetSmallImageUrl(p_user_handle);

	user_info_resp["presence"] = user_info_presence;
	user_info_presence["presence_status"] = ovrUserPresenceStatus_ToString(ovr_User_GetPresenceStatus(p_user_handle));
	user_info_presence["presence_deeplink_message"] = ovr_User_GetPresenceDeeplinkMessage(p_user_handle);
	user_info_presence["presence_destination_api_name"] = ovr_User_GetPresenceDestinationApiName(p_user_handle);
	user_info_presence["presence_lobby_session_id"] = ovr_User_GetPresenceLobbySessionId(p_user_handle);
	user_info_presence["presence_match_session_id"] = ovr_User_GetPresenceMatchSessionId(p_user_handle);

	return user_info_resp;
}

void GDOculusPlatform::_handle_download_update(ovrMessageHandle p_message) {
	if (!ovr_Message_IsError(p_message)) {
		ovrAssetFileDownloadUpdateHandle download_update_handle = ovr_Message_GetAssetFileDownloadUpdate(p_message);

		char native_id[21];
		ovrID asset_id = ovr_AssetFileDownloadUpdate_GetAssetId(download_update_handle);
		ovrID_ToString(native_id, sizeof(native_id), asset_id);

		Dictionary resp;

		String download_asset_id = native_id;
		bool download_completed = ovr_AssetFileDownloadUpdate_GetCompleted(download_update_handle);

		resp["id"] = download_asset_id;
		resp["completed"] = download_completed;
		resp["total_bytes"] = (uint64_t)ovr_AssetFileDownloadUpdate_GetBytesTotalLong(download_update_handle);
		resp["transferred_bytes"] = (int64_t)ovr_AssetFileDownloadUpdate_GetBytesTransferredLong(download_update_handle);

		emit_signal("assetfile_download_update", resp);

		if (download_completed) {
			emit_signal("assetfile_download_finished", download_asset_id);
		}

	} else {
		ovrErrorHandle download_update_err = ovr_Message_GetError(p_message);
		String gd_message = ovr_Error_GetMessage(download_update_err);

		ovrMessageType msg_type = ovr_Message_GetType(p_message);
		String gd_msg_type = ovrMessageType_ToString(msg_type);

		Dictionary gd_msg;
		gd_msg["type"] = gd_msg_type;
		gd_msg["message"] = gd_message;

		emit_signal("unhandled_message", gd_msg);
	}
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
///// OTHER METHODS
/////////////////////////////////////////////////

/// Gets the Java environment. Currently only used for platform initialization
bool GDOculusPlatform::_get_env(JNIEnv **p_env) {
	ERR_FAIL_NULL_V_MSG(jvm, false, "JVM is null");
	jint res = jvm->GetEnv((void **)p_env, JNI_VERSION_1_6);
	if (res == JNI_EDETACHED) {
		res = jvm->AttachCurrentThread(p_env, NULL);
		if (res == JNI_OK)
			return true;
		else {
			*p_env = NULL;
			ERR_FAIL_COND_V(res != JNI_OK, false);
			return false;
		}
	} else
		return false;
}

extern "C" {
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	JNIEnv *env;
	if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
		return JNI_ERR;
	jvm = vm;
	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_org_godot_godotoculusplatform_GodotOculusPlatform_initPlatform(JNIEnv *env, jobject obj, jobject activity) {
	jactivity = reinterpret_cast<jobject>(env->NewGlobalRef(activity));
}
}
