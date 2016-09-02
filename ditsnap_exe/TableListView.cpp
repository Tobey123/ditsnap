#include "stdafx.h"
#include "TableListView.h"
#include "DetailDialog.h"
#include "utility.h"
#include "../EseDataAccess/EseDataAccess.h"

using namespace EseDataAccess;

TableListView::TableListView(EseDbManager* eseDbManager)
	: detailDialog_(nullptr),
	  eseDbManager_(eseDbManager)
{
	eseDbManager_->RegisterTableObserver(this);
	eseDbManager_->RegisterDbObserver(this);
}

TableListView::~TableListView()
{
	CleanupDetailDialog();
	eseDbManager_->RemoveTableObserver(this);
	eseDbManager_->RemoveDbObserver(this);
}

LRESULT TableListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	LRESULT lRet = DefWindowProc();
	SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);
	return lRet;
}

LRESULT TableListView::OnListDoubleClick(LPNMHDR pnmh)
{
	auto pnmia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
	if (pnmia->iItem < 0)
	{
		return 0;
	}

	if (DATATABLE == eseDbManager_->GetCurrentTableName())
	{
		CleanupDetailDialog();
		detailDialog_ = new DetailDialog(eseDbManager_, this,
		                                  listItemIdToEseRowIndex_[pnmia->iItem]);
		detailDialog_->Create(nullptr);
		detailDialog_->ShowWindow(SW_SHOW);
	}
	return 0;
}

void TableListView::LoadTable()
{
	const auto nWidth = 70;
	try
	{
		auto nColumn = eseDbManager_->GetColumnCount();
		for (auto columnIndex = 0; columnIndex < nColumn; ++columnIndex)
		{
			auto columnName = eseDbManager_->GetColumnName(columnIndex);
			InsertColumn(columnIndex, columnName.c_str(), LVCFMT_LEFT, nWidth);
		}

		int rowIndex = 0;
		eseDbManager_->MoveFirstRecord();
		do
		{
			for (auto columnIndex = 0; columnIndex < nColumn; ++columnIndex)
			{
				wstring columnValues;
				auto numberOfColumnValue = eseDbManager_->CountColumnValue(columnIndex);
				for (auto itagSequence = 1; itagSequence <= numberOfColumnValue; ++itagSequence)
				{
					auto columnValue = eseDbManager_->RetrieveColumnDataAsString(columnIndex, itagSequence);
					columnValues += columnValue;
					if (numberOfColumnValue != itagSequence)
					{
						columnValues += L"; ";
					}
				}
				if (columnValues.empty())
				{
					columnValues = NOT_SET;
				}
				AddItem(rowIndex, columnIndex, columnValues.c_str());
			}
			++rowIndex;
		}
		while (eseDbManager_->MoveNextRecord());
	}
	catch (runtime_error& e)
	{
		ShowMessageBox(e.what());
	}
}

void TableListView::LoadDatatable()
{
	columnMap_.clear();
	adNameMap_.clear();
	listItemIdToEseRowIndex_.clear();
	MapColumnNameToColumnIndex(&columnMap_);
	MapColumnNameToAdName(&adNameMap_);
	vector<wstring> listHeaderColumnNames{L"ATTm589825", L"DNT_col", L"PDNT_col", L"cnt_col",
		L"OBJ_col", L"RDNtyp_col", L"NCDNT_col", L"ATTb590606"};
	try
	{
		for (auto i = 0; i < listHeaderColumnNames.size(); ++i)
		{
			int colWidth = i == 0 ? 200 : 70;
			InsertColumnHelper(i, listHeaderColumnNames[i], colWidth);
		}

		int rowIndex = 0;
		eseDbManager_->MoveFirstRecord();
		do
		{
			for (auto i = 0; i < listHeaderColumnNames.size(); ++i)
			{
				AddItemHelper(rowIndex, i, listHeaderColumnNames[i]);
			}
			listItemIdToEseRowIndex_.insert(pair<int, int>(rowIndex, rowIndex));
			++rowIndex;
		}
		while (eseDbManager_->MoveNextRecord());
	}
	catch (runtime_error& e)
	{
		ShowMessageBox(e.what());
	}
}

void TableListView::FilterTable(int filterFlag)
{
	CleanupDetailDialog();
	if (DATATABLE != eseDbManager_->GetCurrentTableName())
	{
		return;
	}

	DeleteAllItems();
	listItemIdToEseRowIndex_.clear();

	wstring classSchemaDnt;
	wstring attributeSchemaDnt;
	wstring subSchemaDnt;
	wstring displaySpecifierDnt;
	try
	{
		eseDbManager_->MoveFirstRecord();
		do
		{
			wstring objectName = RetrieveColumnData(L"ATTm589825");
			if (classSchemaDnt.empty() && L"Class-Schema" == objectName)
			{
				classSchemaDnt = RetrieveColumnData(L"DNT_col");
			}
			if (attributeSchemaDnt.empty() && L"Attribute-Schema" == objectName)
			{
				attributeSchemaDnt = RetrieveColumnData(L"DNT_col");
			}
			if (subSchemaDnt.empty() && L"SubSchema" == objectName)
			{
				subSchemaDnt = RetrieveColumnData(L"DNT_col");
			}
			if (displaySpecifierDnt.empty() && L"Display-Specifier" == objectName)
			{
				displaySpecifierDnt = RetrieveColumnData(L"DNT_col");
			}
		}
		while (eseDbManager_->MoveNextRecord());
	}
	catch (runtime_error& e)
	{
		ShowMessageBox(e.what());
		return;
	}

	vector<wstring> listHeaderColumnNames{L"ATTm589825", L"DNT_col", L"PDNT_col", L"cnt_col",
		L"OBJ_col", L"RDNtyp_col", L"NCDNT_col", L"ATTb590606"};

	try
	{
		int eseRowIndex = -1;
		int rowIndex = 0;
		eseDbManager_->MoveFirstRecord();
		do
		{
			++eseRowIndex;
			wstring objectCategory = RetrieveColumnData(L"ATTb590606");
			if (!(filterFlag & CLASSSCHEMA) && classSchemaDnt == objectCategory) continue;
			if (!(filterFlag & ATTRIBUTESCHEMA) && attributeSchemaDnt == objectCategory) continue;
			if (!(filterFlag & SUBSCHEMA) && subSchemaDnt == objectCategory) continue;
			if (!(filterFlag & DISPLAYSPECIFIER) && displaySpecifierDnt == objectCategory) continue;
			if (!(filterFlag & OTHERS)
				&& classSchemaDnt != objectCategory
				&& attributeSchemaDnt != objectCategory
				&& subSchemaDnt != objectCategory
				&& displaySpecifierDnt != objectCategory
			)
				continue;

			for (int i = 0; i < listHeaderColumnNames.size(); ++i)
			{
				auto s = RetrieveColumnData(listHeaderColumnNames[i]);
				AddItem(rowIndex, i, s.c_str());
			}
			listItemIdToEseRowIndex_.insert(pair<int, int>(rowIndex, eseRowIndex));
			++rowIndex;
		}
		while (eseDbManager_->MoveNextRecord());
	}
	catch (runtime_error& e)
	{
		ShowMessageBox(e.what());
	}
}

wstring TableListView::GetAdNameFromColumnName(wstring columnName)
{
	return adNameMap_[wstring(columnName)];
}

int TableListView::GetColumnIdFromColumnName(wstring columnName)
{
	return columnMap_[wstring(columnName)];
}

void TableListView::LoadEseTable()
{
	CleanupTable();
	CleanupDetailDialog();

	if (DATATABLE == eseDbManager_->GetCurrentTableName())
	{
		LoadDatatable();
	}
	else
	{
		LoadTable();
	}
}

void TableListView::LoadEseDbManager()
{
	CleanupTable();
	CleanupDetailDialog();
}

void TableListView::CleanupTable()
{
	DeleteAllItems();
	int columnCount = GetHeader().GetItemCount();
	for (int i = 0; i < columnCount; ++i)
	{
		DeleteColumn(0);
	}
}

void TableListView::CleanupDetailDialog()
{
	if (nullptr != detailDialog_)
	{
		if (detailDialog_->IsWindow())
		{
			detailDialog_->DestroyWindow();
		}

		delete detailDialog_;
		detailDialog_ = nullptr;
	}
}

void TableListView::InsertColumnHelper(int nCol, wstring columnName, int nWidth)
{
	CString s;
	if (adNameMap_[columnName].empty())
	{
		s.Format(L"%s", columnName.c_str());
	}
	else
	{
		s.Format(L"%s <%s>", columnName.c_str(), adNameMap_[columnName].c_str());
	}
	InsertColumn(nCol, s, LVCFMT_LEFT, nWidth);
}

void TableListView::AddItemHelper(int nItem, int nSubItem, wstring columnName)
{
	wstring s = RetrieveColumnData(columnName);
	AddItem(nItem, nSubItem, s.c_str());
}

wstring TableListView::RetrieveColumnData(wstring columnName)
{
	return eseDbManager_->RetrieveColumnDataAsString(columnMap_[columnName]);
}

bool TableListView::MapColumnNameToColumnIndex(map<wstring, int>* pColumnMap) const
{
	try
	{
		for (uint columnIndex = 0; columnIndex < eseDbManager_->GetColumnCount(); ++columnIndex)
		{
			wstring columnName(eseDbManager_->GetColumnName(columnIndex));
			pColumnMap->insert(pair<wstring, int>(wstring(columnName), columnIndex));
		}
	}
	catch (runtime_error& e)
	{
		ShowMessageBox(e.what());
		return false;
	}
	return true;
}

void TableListView::MapColumnNameToAdName(map<wstring, wstring>* pAdNameMap) const
{
	auto insert = [pAdNameMap](wchar_t* k, wchar_t* v) { pAdNameMap->insert(pair<wstring, wstring>(k, v)); };
	insert(L"ATTq589983", L"ACCOUNT_EXPIRES");
	insert(L"ATTm591131", L"ACCOUNT_NAME_HISTORY");
	insert(L"ATTq590584", L"ACS_AGGREGATE_TOKEN_RATE_PER_USER");
	insert(L"ATTq590590", L"ACS_ALLOCABLE_RSVP_BANDWIDTH");
	insert(L"ATTj590603", L"ACS_CACHE_TIMEOUT");
	insert(L"ATTj590581", L"ACS_DIRECTION");
	insert(L"ATTj590602", L"ACS_DSBM_DEADTIME");
	insert(L"ATTj590600", L"ACS_DSBM_PRIORITY");
	insert(L"ATTj590601", L"ACS_DSBM_REFRESH");
	insert(L"ATTi590594", L"ACS_ENABLE_ACS_SERVICE");
	insert(L"ATTi590723", L"ACS_ENABLE_RSVP_ACCOUNTING");
	insert(L"ATTi590592", L"ACS_ENABLE_RSVP_MESSAGE_LOGGING");
	insert(L"ATTj590593", L"ACS_EVENT_LOG_LEVEL");
	insert(L"ATTm590608", L"ACS_IDENTITY_NAME");
	insert(L"ATTq590721", L"ACS_MAX_AGGREGATE_PEAK_RATE_PER_USER");
	insert(L"ATTj590585", L"ACS_MAX_DURATION_PER_FLOW");
	insert(L"ATTj590725", L"ACS_MAX_NO_OF_ACCOUNT_FILES");
	insert(L"ATTj590598", L"ACS_MAX_NO_OF_LOG_FILES");
	insert(L"ATTq590591", L"ACS_MAX_PEAK_BANDWIDTH");
	insert(L"ATTq590583", L"ACS_MAX_PEAK_BANDWIDTH_PER_FLOW");
	insert(L"ATTj590726", L"ACS_MAX_SIZE_OF_RSVP_ACCOUNT_FILE");
	insert(L"ATTj590599", L"ACS_MAX_SIZE_OF_RSVP_LOG_FILE");
	insert(L"ATTq591137", L"ACS_MAX_TOKEN_BUCKET_PER_FLOW");
	insert(L"ATTq590582", L"ACS_MAX_TOKEN_RATE_PER_FLOW");
	insert(L"ATTq591138", L"ACS_MAXIMUM_SDU_SIZE");
	insert(L"ATTq591141", L"ACS_MINIMUM_DELAY_VARIATION");
	insert(L"ATTq591140", L"ACS_MINIMUM_LATENCY");
	insert(L"ATTq591139", L"ACS_MINIMUM_POLICED_SIZE");
	insert(L"ATTq591144", L"ACS_NON_RESERVED_MAX_SDU_SIZE");
	insert(L"ATTq591145", L"ACS_NON_RESERVED_MIN_POLICED_SIZE");
	insert(L"ATTq591142", L"ACS_NON_RESERVED_PEAK_RATE");
	insert(L"ATTq591143", L"ACS_NON_RESERVED_TOKEN_SIZE");
	insert(L"ATTq590604", L"ACS_NON_RESERVED_TX_LIMIT");
	insert(L"ATTq590722", L"ACS_NON_RESERVED_TX_SIZE");
	insert(L"ATTq590589", L"ACS_PERMISSION_BITS");
	insert(L"ATTm590596", L"ACS_POLICY_NAME");
	insert(L"ATTj590588", L"ACS_PRIORITY");
	insert(L"ATTm590724", L"ACS_RSVP_ACCOUNT_FILES_LOCATION");
	insert(L"ATTm590597", L"ACS_RSVP_LOG_FILES_LOCATION");
	insert(L"ATTj590586", L"ACS_SERVICE_TYPE");
	insert(L"ATTm590580", L"ACS_TIME_OF_DAY");
	insert(L"ATTj590587", L"ACS_TOTAL_NO_OF_FLOWS");
	insert(L"ATTm591136", L"ACS_SERVER_LIST");
	insert(L"ATTm590089", L"ADDITIONAL_INFORMATION");
	insert(L"ATTm590713", L"ADDITIONAL_TRUSTED_SERVICE_NAMES");
	insert(L"ATTm131328", L"ADDRESS");
	insert(L"ATTb591068", L"ADDRESS_BOOK_ROOTS");
	insert(L"ATTk131396", L"ADDRESS_ENTRY_DISPLAY_TABLE");
	insert(L"ATTk131472", L"ADDRESS_ENTRY_DISPLAY_TABLE_MSDOS");
	insert(L"ATTm131689", L"ADDRESS_HOME");
	insert(L"ATTk131327", L"ADDRESS_SYNTAX");
	insert(L"ATTe131422", L"ADDRESS_TYPE");
	insert(L"ATTm590438", L"ADMIN_CONTEXT_MENU");
	insert(L"ATTj589974", L"ADMIN_COUNT");
	insert(L"ATTm131298", L"ADMIN_DESCRIPTION");
	insert(L"ATTm131266", L"ADMIN_DISPLAY_NAME");
	insert(L"ATTm591514", L"ADMIN_MULTISELECT_PROPERTY_PAGES");
	insert(L"ATTm590386", L"ADMIN_PROPERTY_PAGES");
	insert(L"ATTc590737", L"ALLOWED_ATTRIBUTES");
	insert(L"ATTc590738", L"ALLOWED_ATTRIBUTES_EFFECTIVE");
	insert(L"ATTc590735", L"ALLOWED_CHILD_CLASSES");
	insert(L"ATTc590736", L"ALLOWED_CHILD_CLASSES_EFFECTIVE");
	insert(L"ATTm590691", L"ALT_SECURITY_IDENTITIES");
	insert(L"ATTm591032", L"ANR");
	insert(L"ATTj590672", L"APP_SCHEMA_VERSION");
	insert(L"ATTm590042", L"APPLICATION_NAME");
	insert(L"ATTm590165", L"APPLIES_TO");
	insert(L"ATTm590107", L"ASSET_NUMBER");
	insert(L"ATTb590476", L"ASSISTANT");
	insert(L"ATTk591037", L"ASSOC_NT_ACCOUNT");
	insert(L"ATTf1376293", L"ASSOCIATEDDOMAIN");
	insert(L"ATTb1376294", L"ASSOCIATEDNAME");
	insert(L"ATTk58", L"ATTRIBUTECERTIFICATEATTRIBUTE");
	insert(L"ATTm590572", L"ATTRIBUTE_DISPLAY_NAMES");
	insert(L"ATTc131102", L"ATTRIBUTE_ID");
	insert(L"ATTk589973", L"ATTRIBUTE_SECURITY_GUID");
	insert(L"ATTc131104", L"ATTRIBUTE_SYNTAX");
	insert(L"ATTm1572869", L"ATTRIBUTE_TYPES");
	insert(L"ATTk1376311", L"AUDIO");
	insert(L"ATTk590026", L"AUDITING_POLICY");
	insert(L"ATTj589835", L"AUTHENTICATION_OPTIONS");
	insert(L"ATTk38", L"AUTHORITY_REVOCATION_LIST");
	insert(L"ATTc131423", L"AUXILIARY_CLASS");
	insert(L"ATTq589873", L"BAD_PASSWORD_TIME");
	insert(L"ATTj589836", L"BAD_PWD_COUNT");
	insert(L"ATTk590156", L"BIRTH_LOCATION");
	insert(L"ATTb590644", L"BRIDGEHEAD_SERVER_LIST_BL");
	insert(L"ATTb590643", L"BRIDGEHEAD_TRANSPORT_LIST");
	insert(L"ATTm1376304", L"BUILDINGNAME");
	insert(L"ATTq589837", L"BUILTIN_CREATION_TIME");
	insert(L"ATTq589838", L"BUILTIN_MODIFIED_COUNT");
	insert(L"ATTm15", L"BUSINESS_CATEGORY");
	insert(L"ATTj590108", L"BYTES_PER_MINUTE");
	insert(L"ATTk37", L"CA_CERTIFICATE");
	insert(L"ATTm590521", L"CA_CERTIFICATE_DN");
	insert(L"ATTm590511", L"CA_CONNECT");
	insert(L"ATTm590514", L"CA_USAGES");
	insert(L"ATTm590512", L"CA_WEB_URL");
	insert(L"ATTm590639", L"CAN_UPGRADE_SCRIPT");
	insert(L"ATTm590740", L"CANONICAL_NAME");
	insert(L"ATTm1441793", L"CARLICENSE");
	insert(L"ATTm590499", L"CATALOGS");
	insert(L"ATTm590496", L"CATEGORIES");
	insert(L"ATTk590146", L"CATEGORY_ID");
	insert(L"ATTb590508", L"CERTIFICATE_AUTHORITY_OBJECT");
	insert(L"ATTk39", L"CERTIFICATE_REVOCATION_LIST");
	insert(L"ATTm590647", L"CERTIFICATE_TEMPLATES");
	insert(L"ATTm590434", L"CLASS_DISPLAY_NAME");
	insert(L"ATTj589840", L"CODE_PAGE");
	insert(L"ATTm589843", L"COM_CLASSID");
	insert(L"ATTm590073", L"COM_CLSID");
	insert(L"ATTm589844", L"COM_INTERFACEID");
	insert(L"ATTm590077", L"COM_OTHER_PROG_ID");
	insert(L"ATTm589845", L"COM_PROGID");
	insert(L"ATTm590075", L"COM_TREAT_AS_CLASS_ID");
	insert(L"ATTm590078", L"COM_TYPELIB_ID");
	insert(L"ATTm590074", L"COM_UNIQUE_LIBID");
	insert(L"ATTm131153", L"COMMENT");
	insert(L"ATTm3", L"COMMON_NAME");
	insert(L"ATTm131218", L"COMPANY");
	insert(L"ATTi589848", L"CONTENT_INDEXING_ALLOWED");
	insert(L"ATTm590323", L"CONTEXT_MENU");
	insert(L"ATTk590024", L"CONTROL_ACCESS_RIGHTS");
	insert(L"ATTj131207", L"COST");
	insert(L"ATTj589849", L"COUNTRY_CODE");
	insert(L"ATTm6", L"COUNTRY_NAME");
	insert(L"ATTm590634", L"CREATE_DIALOG");
	insert(L"ATTl1638401", L"CREATE_TIME_STAMP");
	insert(L"ATTm590636", L"CREATE_WIZARD_EXT");
	insert(L"ATTq589850", L"CREATION_TIME");
	insert(L"ATTm590322", L"CREATION_WIZARD");
	insert(L"ATTm590503", L"CREATOR");
	insert(L"ATTb590513", L"CRL_OBJECT");
	insert(L"ATTk590507", L"CRL_PARTITIONED_REVOCATION_LIST");
	insert(L"ATTk40", L"CROSS_CERTIFICATE_PAIR");
	insert(L"ATTk590161", L"CURR_MACHINE_ID");
	insert(L"ATTk590159", L"CURRENT_LOCATION");
	insert(L"ATTb590520", L"CURRENT_PARENT_CA");
	insert(L"ATTk589851", L"CURRENT_VALUE");
	insert(L"ATTk589879", L"DBCS_PWD");
	insert(L"ATTb590037", L"DEFAULT_CLASS_STORE");
	insert(L"ATTb590304", L"DEFAULT_GROUP");
	insert(L"ATTi590342", L"DEFAULT_HIDING_VALUE");
	insert(L"ATTb589881", L"DEFAULT_LOCAL_POLICY_OBJECT");
	insert(L"ATTb590607", L"DEFAULT_OBJECT_CATEGORY");
	insert(L"ATTj590056", L"DEFAULT_PRIORITY");
	insert(L"ATTm590048", L"DEFAULT_SECURITY_DESCRIPTOR");
	insert(L"ATTk53", L"DELTA_REVOCATION_LIST");
	insert(L"ATTm131213", L"DEPARTMENT");
	insert(L"ATTm1441794", L"DEPARTMENTNUMBER");
	insert(L"ATTm13", L"DESCRIPTION");
	insert(L"ATTm590170", L"DESKTOP_PROFILE");
	insert(L"ATTf27", L"DESTINATION_INDICATOR");
	insert(L"ATTk590539", L"DHCP_CLASSES");
	insert(L"ATTq590524", L"DHCP_FLAGS");
	insert(L"ATTm590525", L"DHCP_IDENTIFICATION");
	insert(L"ATTf590530", L"DHCP_MASK");
	insert(L"ATTq590543", L"DHCP_MAXKEY");
	insert(L"ATTm590527", L"DHCP_OBJ_DESCRIPTION");
	insert(L"ATTm590526", L"DHCP_OBJ_NAME");
	insert(L"ATTk590538", L"DHCP_OPTIONS");
	insert(L"ATTk590542", L"DHCP_PROPERTIES");
	insert(L"ATTf590531", L"DHCP_RANGES");
	insert(L"ATTf590533", L"DHCP_RESERVATIONS");
	insert(L"ATTf590528", L"DHCP_SERVERS");
	insert(L"ATTf590532", L"DHCP_SITES");
	insert(L"ATTf590541", L"DHCP_STATE");
	insert(L"ATTf590529", L"DHCP_SUBNETS");
	insert(L"ATTj590523", L"DHCP_TYPE");
	insert(L"ATTq590522", L"DHCP_UNIQUE_KEY");
	insert(L"ATTq590544", L"DHCP_UPDATE_TIME");
	insert(L"ATTm131085", L"DISPLAY_NAME");
	insert(L"ATTf131425", L"DISPLAY_NAME_PRINTABLE");
	insert(L"ATTm1572866", L"DIT_CONTENT_RULES");
	insert(L"ATTm590085", L"DIVISION");
	insert(L"ATTb131108", L"DMD_LOCATION");
	insert(L"ATTm131670", L"DMD_NAME");
	insert(L"ATTb591066", L"DN_REFERENCE_UPDATE");
	insert(L"ATTi590202", L"DNS_ALLOW_DYNAMIC");
	insert(L"ATTi590203", L"DNS_ALLOW_XFR");
	insert(L"ATTm590443", L"DNS_HOST_NAME");
	insert(L"ATTj590205", L"DNS_NOTIFY_SECONDARIES");
	insert(L"ATTk591130", L"DNS_PROPERTY");
	insert(L"ATTk590206", L"DNS_RECORD");
	insert(L"ATTm589852", L"DNS_ROOT");
	insert(L"ATTj590204", L"DNS_SECURE_SECONDARIES");
	insert(L"ATTi591238", L"DNS_TOMBSTONED");
	insert(L"ATTb1376270", L"DOCUMENTAUTHOR");
	insert(L"ATTm1376267", L"DOCUMENTIDENTIFIER");
	insert(L"ATTm1376271", L"DOCUMENTLOCATION");
	insert(L"ATTm1376312", L"DOCUMENTPUBLISHER");
	insert(L"ATTm1376268", L"DOCUMENTTITLE");
	insert(L"ATTm1376269", L"DOCUMENTVERSION");
	insert(L"ATTb590492", L"DOMAIN_CERTIFICATE_AUTHORITIES");
	insert(L"ATTm1376281", L"DOMAIN_COMPONENT");
	insert(L"ATTb590296", L"DOMAIN_CROSS_REF");
	insert(L"ATTb590510", L"DOMAIN_ID");
	insert(L"ATTj590579", L"DOMAIN_IDENTIFIER");
	insert(L"ATTb589856", L"DOMAIN_POLICY_OBJECT");
	insert(L"ATTb590246", L"DOMAIN_POLICY_REFERENCE");
	insert(L"ATTm589982", L"DOMAIN_REPLICA");
	insert(L"ATTk590245", L"DOMAIN_WIDE_POLICY");
	insert(L"ATTm1376261", L"DRINK");
	insert(L"ATTm590053", L"DRIVER_NAME");
	insert(L"ATTj590100", L"DRIVER_VERSION");
	insert(L"ATTl591181", L"DS_CORE_PROPAGATION_DATA");
	insert(L"ATTm131284", L"DS_HEURISTICS");
	insert(L"ATTj591168", L"DS_UI_ADMIN_MAXIMUM");
	insert(L"ATTm591167", L"DS_UI_ADMIN_NOTIFICATION");
	insert(L"ATTj591169", L"DS_UI_SHELL_MAXIMUM");
	insert(L"ATTk131146", L"DSA_SIGNATURE");
	insert(L"ATTb590361", L"DYNAMIC_LDAP_SERVER");
	insert(L"ATTm1376259", L"E_MAIL_ADDRESSES");
	insert(L"ATTk590092", L"EFSPOLICY");
	insert(L"ATTm589859", L"EMPLOYEE_ID");
	insert(L"ATTm131682", L"EMPLOYEE_NUMBER");
	insert(L"ATTm131685", L"EMPLOYEE_TYPE");
	insert(L"ATTi131629", L"ENABLED");
	insert(L"ATTi589860", L"ENABLED_CONNECTION");
	insert(L"ATTm590649", L"ENROLLMENT_PROVIDERS");
	insert(L"ATTm590733", L"EXTENDED_ATTRIBUTE_INFO");
	insert(L"ATTi131452", L"EXTENDED_CHARS_ALLOWED");
	insert(L"ATTm590732", L"EXTENDED_CLASS_INFO");
	insert(L"ATTm131299", L"EXTENSION_NAME");
	insert(L"ATTm591511", L"EXTRA_COLUMNS");
	insert(L"ATTm23", L"FACSIMILE_TELEPHONE_NUMBER");
	insert(L"ATTm590640", L"FILE_EXT_PRIORITY");
	insert(L"ATTj589862", L"FLAGS");
	insert(L"ATTm590335", L"FLAT_NAME");
	insert(L"ATTq589863", L"FORCE_LOGOFF");
	insert(L"ATTk590180", L"FOREIGN_IDENTIFIER");
	insert(L"ATTm590506", L"FRIENDLY_NAMES");
	insert(L"ATTi590734", L"FROM_ENTRY");
	insert(L"ATTb589864", L"FROM_SERVER");
	insert(L"ATTb590693", L"FRS_COMPUTER_REFERENCE");
	insert(L"ATTb590694", L"FRS_COMPUTER_REFERENCE_BL");
	insert(L"ATTm590695", L"FRS_CONTROL_DATA_CREATION");
	insert(L"ATTm590696", L"FRS_CONTROL_INBOUND_BACKLOG");
	insert(L"ATTm590697", L"FRS_CONTROL_OUTBOUND_BACKLOG");
	insert(L"ATTm590308", L"FRS_DIRECTORY_FILTER");
	insert(L"ATTj590314", L"FRS_DS_POLL");
	insert(L"ATTk590360", L"FRS_EXTENSIONS");
	insert(L"ATTm590315", L"FRS_FAULT_CONDITION");
	insert(L"ATTm590307", L"FRS_FILE_FILTER");
	insert(L"ATTj590698", L"FRS_FLAGS");
	insert(L"ATTj590358", L"FRS_LEVEL_LIMIT");
	insert(L"ATTb590699", L"FRS_MEMBER_REFERENCE");
	insert(L"ATTb590700", L"FRS_MEMBER_REFERENCE_BL");
	insert(L"ATTj590701", L"FRS_PARTNER_AUTH_LEVEL");
	insert(L"ATTb590702", L"FRS_PRIMARY_MEMBER");
	insert(L"ATTk590357", L"FRS_REPLICA_SET_GUID");
	insert(L"ATTj589855", L"FRS_REPLICA_SET_TYPE");
	insert(L"ATTm590311", L"FRS_ROOT_PATH");
	insert(L"ATTp590359", L"FRS_ROOT_SECURITY");
	insert(L"ATTm590324", L"FRS_SERVICE_COMMAND");
	insert(L"ATTm590703", L"FRS_SERVICE_COMMAND_STATUS");
	insert(L"ATTm590312", L"FRS_STAGING_PATH");
	insert(L"ATTl590704", L"FRS_TIME_LAST_COMMAND");
	insert(L"ATTl590705", L"FRS_TIME_LAST_CONFIG_CHANGE");
	insert(L"ATTj590309", L"FRS_UPDATE_TIMEOUT");
	insert(L"ATTm590706", L"FRS_VERSION");
	insert(L"ATTk589867", L"FRS_VERSION_GUID");
	insert(L"ATTm590310", L"FRS_WORKING_PATH");
	insert(L"ATTb590193", L"FSMO_ROLE_OWNER");
	insert(L"ATTj131373", L"GARBAGE_COLL_PERIOD");
	insert(L"ATTi589865", L"GENERATED_CONNECTION");
	insert(L"ATTm44", L"GENERATION_QUALIFIER");
	insert(L"ATTm42", L"GIVEN_NAME");
	insert(L"ATTb591069", L"GLOBAL_ADDRESS_LIST");
	insert(L"ATTc131094", L"GOVERNS_ID");
	insert(L"ATTm590715", L"GP_LINK");
	insert(L"ATTj590716", L"GP_OPTIONS");
	insert(L"ATTm590718", L"GPC_FILE_SYS_PATH");
	insert(L"ATTj590717", L"GPC_FUNCTIONALITY_VERSION");
	insert(L"ATTm591172", L"GPC_MACHINE_EXTENSION_NAMES");
	insert(L"ATTm591173", L"GPC_USER_EXTENSION_NAMES");
	insert(L"ATTm591518", L"GPC_WQL_FILTER");
	insert(L"ATTj589976", L"GROUP_ATTRIBUTES");
	insert(L"ATTk589990", L"GROUP_MEMBERSHIP_SAM");
	insert(L"ATTm590169", L"GROUP_PRIORITY");
	insert(L"ATTj590574", L"GROUP_TYPE");
	insert(L"ATTm590168", L"GROUPS_TO_IGNORE");
	insert(L"ATTb131086", L"HAS_MASTER_NCS");
	insert(L"ATTb131087", L"HAS_PARTIAL_REPLICA_NCS");
	insert(L"ATTk131474", L"HELP_DATA16");
	insert(L"ATTk131081", L"HELP_DATA32");
	insert(L"ATTm131399", L"HELP_FILE_NAME");
	insert(L"ATTi591604", L"HIDE_FROM_AB");
	insert(L"ATTm589868", L"HOME_DIRECTORY");
	insert(L"ATTm589869", L"HOME_DRIVE");
	insert(L"ATTm51", L"HOUSEIDENTIFIER");
	insert(L"ATTm1376265", L"HOST");
	insert(L"ATTm590043", L"ICON_PATH");
	insert(L"ATTk590144", L"IMPLEMENTED_CATEGORIES");
	insert(L"ATTm590505", L"INDEXEDSCOPES");
	insert(L"ATTm590363", L"INITIAL_AUTH_INCOMING");
	insert(L"ATTm590364", L"INITIAL_AUTH_OUTGOING");
	insert(L"ATTm43", L"INITIALS");
	insert(L"ATTj590671", L"INSTALL_UI_LEVEL");
	insert(L"ATTj131073", L"INSTANCE_TYPE");
	insert(L"ATTj591072", L"INTER_SITE_TOPOLOGY_FAILOVER");
	insert(L"ATTb591070", L"INTER_SITE_TOPOLOGY_GENERATOR");
	insert(L"ATTj591071", L"INTER_SITE_TOPOLOGY_RENEW");
	insert(L"ATTg25", L"INTERNATIONAL_ISDN_NUMBER");
	insert(L"ATTk131187", L"INVOCATION_ID");
	insert(L"ATTk590447", L"IPSEC_DATA");
	insert(L"ATTj590446", L"IPSEC_DATA_TYPE");
	insert(L"ATTb590453", L"IPSEC_FILTER_REFERENCE");
	insert(L"ATTm590445", L"IPSEC_ID");
	insert(L"ATTb590450", L"IPSEC_ISAKMP_REFERENCE");
	insert(L"ATTm590444", L"IPSEC_NAME");
	insert(L"ATTm590712", L"IPSEC_NEGOTIATION_POLICY_ACTION");
	insert(L"ATTb590452", L"IPSEC_NEGOTIATION_POLICY_REFERENCE");
	insert(L"ATTm590711", L"IPSEC_NEGOTIATION_POLICY_TYPE");
	insert(L"ATTb590451", L"IPSEC_NFA_REFERENCE");
	insert(L"ATTb590448", L"IPSEC_OWNERS_REFERENCE");
	insert(L"ATTb590341", L"IPSEC_POLICY_REFERENCE");
	insert(L"ATTi590692", L"IS_CRITICAL_SYSTEM_OBJECT");
	insert(L"ATTi590485", L"IS_DEFUNCT");
	insert(L"ATTi131120", L"IS_DELETED");
	insert(L"ATTi591036", L"IS_EPHEMERAL");
	insert(L"ATTb131174", L"IS_MEMBER_OF_DL");
	insert(L"ATTi590463", L"IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET");
	insert(L"ATTb590462", L"IS_PRIVILEGE_HOLDER");
	insert(L"ATTi131105", L"IS_SINGLE_VALUED");
	insert(L"ATTk1376316", L"JPEGPHOTO");
	insert(L"ATTm589872", L"KEYWORDS");
	insert(L"ATTe2", L"KNOWLEDGE_INFORMATION");
	insert(L"ATTq590343", L"LAST_BACKUP_RESTORATION_TIME");
	insert(L"ATTq589874", L"LAST_CONTENT_INDEXED");
	insert(L"ATTb590605", L"LAST_KNOWN_PARENT");
	insert(L"ATTq589875", L"LAST_LOGOFF");
	insert(L"ATTq589876", L"LAST_LOGON");
	insert(L"ATTq591520", L"LAST_LOGON_TIMESTAMP");
	insert(L"ATTq589877", L"LAST_SET_TIME");
	insert(L"ATTm590154", L"LAST_UPDATE_SEQUENCE");
	insert(L"ATTm590667", L"LDAP_ADMIN_LIMITS");
	insert(L"ATTm131532", L"LDAP_DISPLAY_NAME");
	insert(L"ATTk590668", L"LDAP_IPDENY_LIST");
	insert(L"ATTe590479", L"LEGACY_EXCHANGE_DN");
	insert(L"ATTj131122", L"LINK_ID");
	insert(L"ATTk590093", L"LINK_TRACK_SECRET");
	insert(L"ATTk589984", L"LM_PWD_HISTORY");
	insert(L"ATTj589880", L"LOCAL_POLICY_FLAGS");
	insert(L"ATTb590281", L"LOCAL_POLICY_REFERENCE");
	insert(L"ATTj589882", L"LOCALE_ID");
	insert(L"ATTm7", L"LOCALITY_NAME");
	insert(L"ATTm590641", L"LOCALIZED_DESCRIPTION");
	insert(L"ATTj591177", L"LOCALIZATION_DISPLAY_ID");
	insert(L"ATTm590046", L"LOCATION");
	insert(L"ATTq589885", L"LOCK_OUT_OBSERVATION_WINDOW");
	insert(L"ATTq589884", L"LOCKOUT_DURATION");
	insert(L"ATTj589897", L"LOCKOUT_THRESHOLD");
	insert(L"ATTq590486", L"LOCKOUT_TIME");
	insert(L"ATTk1441828", L"LOGO");
	insert(L"ATTj589993", L"LOGON_COUNT");
	insert(L"ATTk589888", L"LOGON_HOURS");
	insert(L"ATTk589889", L"LOGON_WORKSTATION");
	insert(L"ATTq589890", L"LSA_CREATION_TIME");
	insert(L"ATTq589891", L"LSA_MODIFIED_COUNT");
	insert(L"ATTj589892", L"MACHINE_ARCHITECTURE");
	insert(L"ATTq590344", L"MACHINE_PASSWORD_CHANGE_INTERVAL");
	insert(L"ATTj589895", L"MACHINE_ROLE");
	insert(L"ATTk590283", L"MACHINE_WIDE_POLICY");
	insert(L"ATTb590477", L"MANAGED_BY");
	insert(L"ATTb590478", L"MANAGED_OBJECTS");
	insert(L"ATTb1376266", L"MANAGER");
	insert(L"ATTj131121", L"MAPI_ID");
	insert(L"ATTk589896", L"MARSHALLED_INTERFACE");
	insert(L"ATTb591233", L"MASTERED_BY");
	insert(L"ATTq589898", L"MAX_PWD_AGE");
	insert(L"ATTq589899", L"MAX_RENEW_AGE");
	insert(L"ATTq589900", L"MAX_STORAGE");
	insert(L"ATTq589901", L"MAX_TICKET_AGE");
	insert(L"ATTc131097", L"MAY_CONTAIN");
	insert(L"ATTm590406", L"MEETINGADVERTISESCOPE");
	insert(L"ATTm590397", L"MEETINGAPPLICATION");
	insert(L"ATTj590413", L"MEETINGBANDWIDTH");
	insert(L"ATTk590414", L"MEETINGBLOB");
	insert(L"ATTm590402", L"MEETINGCONTACTINFO");
	insert(L"ATTm590391", L"MEETINGDESCRIPTION");
	insert(L"ATTl590412", L"MEETINGENDTIME");
	insert(L"ATTm590389", L"MEETINGID");
	insert(L"ATTm590404", L"MEETINGIP");
	insert(L"ATTm590409", L"MEETINGISENCRYPTED");
	insert(L"ATTm590392", L"MEETINGKEYWORD");
	insert(L"ATTm590398", L"MEETINGLANGUAGE");
	insert(L"ATTm590393", L"MEETINGLOCATION");
	insert(L"ATTj590400", L"MEETINGMAXPARTICIPANTS");
	insert(L"ATTm590390", L"MEETINGNAME");
	insert(L"ATTm590401", L"MEETINGORIGINATOR");
	insert(L"ATTm590403", L"MEETINGOWNER");
	insert(L"ATTm590394", L"MEETINGPROTOCOL");
	insert(L"ATTm590408", L"MEETINGRATING");
	insert(L"ATTm590410", L"MEETINGRECURRENCE");
	insert(L"ATTm590405", L"MEETINGSCOPE");
	insert(L"ATTl590411", L"MEETINGSTARTTIME");
	insert(L"ATTm590395", L"MEETINGTYPE");
	insert(L"ATTm590407", L"MEETINGURL");
	insert(L"ATTb31", L"MEMBER");
	insert(L"ATTm590474", L"MHS_OR_ADDRESS");
	insert(L"ATTq589902", L"MIN_PWD_AGE");
	insert(L"ATTj589903", L"MIN_PWD_LENGTH");
	insert(L"ATTq589904", L"MIN_TICKET_AGE");
	insert(L"ATTq589992", L"MODIFIED_COUNT");
	insert(L"ATTq589905", L"MODIFIED_COUNT_AT_LAST_PROM");
	insert(L"ATTl1638402", L"MODIFY_TIME_STAMP");
	insert(L"ATTk589906", L"MONIKER");
	insert(L"ATTm589907", L"MONIKER_DISPLAY_NAME");
	insert(L"ATTk591129", L"MOVE_TREE_STATE");
	insert(L"ATTb591251", L"MS_COM_DEFAULTPARTITIONLINK");
	insert(L"ATTk591252", L"MS_COM_OBJECTID");
	insert(L"ATTb591247", L"MS_COM_PARTITIONLINK");
	insert(L"ATTb591248", L"MS_COM_PARTITIONSETLINK");
	insert(L"ATTb591249", L"MS_COM_USERLINK");
	insert(L"ATTb591250", L"MS_COM_USERPARTITIONSETLINK");
	insert(L"ATTk591667", L"MS_DRM_IDENTITY_CERTIFICATE");
	insert(L"ATTm591541", L"MS_DS_ADDITIONAL_DNS_HOST_NAME");
	insert(L"ATTm591542", L"MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME");
	insert(L"ATTj591613", L"MS_DS_ALL_USERS_TRUST_QUOTA");
	insert(L"ATTm591534", L"MS_DS_ALLOWED_DNS_SUFFIXES");
	insert(L"ATTm591611", L"MS_DS_ALLOWED_TO_DELEGATE_TO");
	insert(L"ATTc591282", L"MS_DS_AUXILIARY_CLASSES");
	insert(L"ATTj591493", L"MS_DS_APPROX_IMMED_SUBORDINATES");
	insert(L"ATTm591643", L"MS_DS_AZ_APPLICATION_DATA");
	insert(L"ATTm591622", L"MS_DS_AZ_APPLICATION_NAME");
	insert(L"ATTm591641", L"MS_DS_AZ_APPLICATION_VERSION");
	insert(L"ATTm591625", L"MS_DS_AZ_BIZ_RULE");
	insert(L"ATTm591626", L"MS_DS_AZ_BIZ_RULE_LANGUAGE");
	insert(L"ATTm591640", L"MS_DS_AZ_CLASS_ID");
	insert(L"ATTj591619", L"MS_DS_AZ_DOMAIN_TIMEOUT");
	insert(L"ATTi591629", L"MS_DS_AZ_GENERATE_AUDITS");
	insert(L"ATTm591627", L"MS_DS_AZ_LAST_IMPORTED_BIZ_RULE_PATH");
	insert(L"ATTm591616", L"MS_DS_AZ_LDAP_QUERY");
	insert(L"ATTj591648", L"MS_DS_AZ_MAJOR_VERSION");
	insert(L"ATTj591649", L"MS_DS_AZ_MINOR_VERSION");
	insert(L"ATTj591624", L"MS_DS_AZ_OPERATION_ID");
	insert(L"ATTm591623", L"MS_DS_AZ_SCOPE_NAME");
	insert(L"ATTj591620", L"MS_DS_AZ_SCRIPT_ENGINE_CACHE_MAX");
	insert(L"ATTj591621", L"MS_DS_AZ_SCRIPT_TIMEOUT");
	insert(L"ATTi591642", L"MS_DS_AZ_TASK_IS_ROLE_DEFINITION");
	insert(L"ATTj591283", L"MS_DS_BEHAVIOR_VERSION");
	insert(L"ATTk591655", L"MS_DS_BYTE_ARRAY");
	insert(L"ATTk591265", L"MS_DS_CACHED_MEMBERSHIP");
	insert(L"ATTq591266", L"MS_DS_CACHED_MEMBERSHIP_TIME_STAMP");
	insert(L"ATTk591184", L"MS_DS_CONSISTENCY_GUID");
	insert(L"ATTj591185", L"MS_DS_CONSISTENCY_CHILD_COUNT");
	insert(L"ATTr591234", L"MS_DS_CREATOR_SID");
	insert(L"ATTl591656", L"MS_DS_DATE_TIME");
	insert(L"ATTj591670", L"MS_DS_DEFAULT_QUOTA");
	insert(L"ATTm591543", L"MS_DS_DNSROOTALIAS");
	insert(L"ATTl591446", L"MS_DS_ENTRY_TIME_TO_DIE");
	insert(L"ATTk591607", L"MS_DS_EXECUTESCRIPTPASSWORD");
	insert(L"ATTm591657", L"MS_DS_EXTERNAL_KEY");
	insert(L"ATTm591658", L"MS_DS_EXTERNAL_STORE");
	insert(L"ATTm591527", L"MS_DS_FILTER_CONTAINERS");
	insert(L"ATTh591533", L"MS_DS_HAS_INSTANTIATED_NCS");
	insert(L"ATTb591644", L"MS_DS_HAS_DOMAIN_NCS");
	insert(L"ATTb591660", L"MS_DS_HAS_MASTER_NCS");
	insert(L"ATTj591659", L"MS_DS_INTEGER");
	insert(L"ATTj591540", L"MS_DS_INTID");
	insert(L"ATTj591606", L"MS_DS_KEYVERSIONNUMBER");
	insert(L"ATTj591608", L"MS_DS_LOGON_TIME_SYNC_INTERVAL");
	insert(L"ATTb591661", L"MS_DS_MASTERED_BY");
	insert(L"ATTj591666", L"MS_DS_MAX_VALUES");
	insert(L"ATTb591630", L"MS_DS_MEMBERS_FOR_AZ_ROLE");
	insert(L"ATTb591631", L"MS_DS_MEMBERS_FOR_AZ_ROLE_BL");
	insert(L"ATTb591617", L"MS_DS_NON_MEMBERS");
	insert(L"ATTb591618", L"MS_DS_NON_MEMBERS_BL");
	insert(L"ATTk591526", L"MS_DS_TRUST_FOREST_TRUST_INFO");
	insert(L"ATTj591671", L"MS_DS_TOMBSTONE_QUOTA_FACTOR");
	insert(L"ATTm591674", L"MS_DS_TOP_QUOTA_USAGE");
	insert(L"ATTj591235", L"MS_DS_MACHINE_ACCOUNT_QUOTA");
	insert(L"ATTb591664", L"MS_DS_OBJECT_REFERENCE");
	insert(L"ATTb591665", L"MS_DS_OBJECT_REFERENCE_BL");
	insert(L"ATTb591636", L"MS_DS_OPERATIONS_FOR_AZ_ROLE");
	insert(L"ATTb591637", L"MS_DS_OPERATIONS_FOR_AZ_ROLE_BL");
	insert(L"ATTb591632", L"MS_DS_OPERATIONS_FOR_AZ_TASK");
	insert(L"ATTb591633", L"MS_DS_OPERATIONS_FOR_AZ_TASK_BL");
	insert(L"ATTm591445", L"MS_DS_OTHER_SETTINGS");
	insert(L"ATTj591669", L"MS_DS_QUOTA_AMOUNT");
	insert(L"ATTj591672", L"MS_DS_QUOTA_EFFECTIVE");
	insert(L"ATTr591668", L"MS_DS_QUOTA_TRUSTEE");
	insert(L"ATTj591673", L"MS_DS_QUOTA_USED");
	insert(L"ATTm591528", L"MS_DS_NC_REPL_CURSORS");
	insert(L"ATTm591529", L"MS_DS_NC_REPL_INBOUND_NEIGHBORS");
	insert(L"ATTm591530", L"MS_DS_NC_REPL_OUTBOUND_NEIGHBORS");
	insert(L"ATTb591485", L"MS_DS_NC_REPLICA_LOCATIONS");
	insert(L"ATTm591513", L"MS_DS_NON_SECURITY_GROUP_EXTRA_CLASSES");
	insert(L"ATTj591612", L"MS_DS_PER_USER_TRUST_QUOTA");
	insert(L"ATTj591614", L"MS_DS_PER_USER_TRUST_TOMBSTONES_QUOTA");
	insert(L"ATTb591268", L"MS_DS_PREFERRED_GC_SITE");
	insert(L"ATTm591531", L"MS_DS_REPL_ATTRIBUTE_META_DATA");
	insert(L"ATTm591532", L"MS_DS_REPL_VALUE_META_DATA");
	insert(L"ATTh591232", L"MS_DS_REPLICATES_NC_REASON");
	insert(L"ATTj591487", L"MS_DS_REPLICATION_NOTIFY_FIRST_DSA_DELAY");
	insert(L"ATTj591488", L"MS_DS_REPLICATION_NOTIFY_SUBSEQUENT_DSA_DELAY");
	insert(L"ATTj591544", L"MS_DS_REPLICATIONEPOCH");
	insert(L"ATTk591650", L"MS_DS_RETIRED_REPL_NC_SIGNATURES");
	insert(L"ATTk591264", L"MS_DS_SCHEMA_EXTENSIONS");
	insert(L"ATTb591535", L"MS_DS_SD_REFERENCE_DOMAIN");
	insert(L"ATTm591512", L"MS_DS_SECURITY_GROUP_EXTRA_CLASSES");
	insert(L"ATTm591521", L"MS_DS_SETTINGS");
	insert(L"ATTk591267", L"MS_DS_SITE_AFFINITY");
	insert(L"ATTm591539", L"MS_DS_SPN_SUFFIXES");
	insert(L"ATTb591638", L"MS_DS_TASKS_FOR_AZ_ROLE");
	insert(L"ATTb591639", L"MS_DS_TASKS_FOR_AZ_ROLE_BL");
	insert(L"ATTb591634", L"MS_DS_TASKS_FOR_AZ_TASK");
	insert(L"ATTb591635", L"MS_DS_TASKS_FOR_AZ_TASK_BL");
	insert(L"ATTj591284", L"MS_DS_USER_ACCOUNT_CONTROL_COMPUTED");
	insert(L"ATTm591545", L"MS_DS_UPDATESCRIPT");
	insert(L"ATTm131516", L"MS_EXCH_ASSISTANT_NAME");
	insert(L"ATTm131668", L"MS_EXCH_HOUSE_IDENTIFIER");
	insert(L"ATTm131665", L"MS_EXCH_LABELEDURI");
	insert(L"ATTb131176", L"MS_EXCH_OWNER_BL");
	insert(L"ATTb591517", L"MS_FRS_HUB_MEMBER");
	insert(L"ATTm591516", L"MS_FRS_TOPOLOGY_PREF");
	insert(L"ATTk591645", L"MS_IEEE_80211_DATA");
	insert(L"ATTj591646", L"MS_IEEE_80211_DATA_TYPE");
	insert(L"ATTm591647", L"MS_IEEE_80211_ID");
	insert(L"ATTm591610", L"MS_IIS_FTP_DIR");
	insert(L"ATTm591609", L"MS_IIS_FTP_ROOT");
	insert(L"ATTm591260", L"MS_PKI_CERT_TEMPLATE_OID");
	insert(L"ATTm591498", L"MS_PKI_CERTIFICATE_APPLICATION_POLICY");
	insert(L"ATTj591256", L"MS_PKI_CERTIFICATE_NAME_FLAG");
	insert(L"ATTm591263", L"MS_PKI_CERTIFICATE_POLICY");
	insert(L"ATTj591254", L"MS_PKI_ENROLLMENT_FLAG");
	insert(L"ATTj591257", L"MS_PKI_MINIMAL_KEY_SIZE");
	insert(L"ATTj591495", L"MS_PKI_OID_ATTRIBUTE");
	insert(L"ATTm591496", L"MS_PKI_OID_CPS");
	insert(L"ATTm591536", L"MS_PKI_OID_LOCALIZEDNAME");
	insert(L"ATTm591497", L"MS_PKI_OID_USER_NOTICE");
	insert(L"ATTj591255", L"MS_PKI_PRIVATE_KEY_FLAG");
	insert(L"ATTm591261", L"MS_PKI_SUPERSEDE_TEMPLATES");
	insert(L"ATTj591259", L"MS_PKI_TEMPLATE_MINOR_REVISION");
	insert(L"ATTj591258", L"MS_PKI_TEMPLATE_SCHEMA_VERSION");
	insert(L"ATTm591499", L"MS_PKI_RA_APPLICATION_POLICIES");
	insert(L"ATTm591262", L"MS_PKI_RA_POLICIES");
	insert(L"ATTj591253", L"MS_PKI_RA_SIGNATURE");
	insert(L"ATTm590708", L"MS_RRAS_ATTRIBUTE");
	insert(L"ATTm590707", L"MS_RRAS_VENDOR_ATTRIBUTE_ENTRY");
	insert(L"ATTm591187", L"MS_SQL_NAME");
	insert(L"ATTm591188", L"MS_SQL_REGISTEREDOWNER");
	insert(L"ATTm591189", L"MS_SQL_CONTACT");
	insert(L"ATTm591190", L"MS_SQL_LOCATION");
	insert(L"ATTq591191", L"MS_SQL_MEMORY");
	insert(L"ATTj591192", L"MS_SQL_BUILD");
	insert(L"ATTm591193", L"MS_SQL_SERVICEACCOUNT");
	insert(L"ATTj591194", L"MS_SQL_CHARACTERSET");
	insert(L"ATTm591195", L"MS_SQL_SORTORDER");
	insert(L"ATTj591196", L"MS_SQL_UNICODESORTORDER");
	insert(L"ATTi591197", L"MS_SQL_CLUSTERED");
	insert(L"ATTm591198", L"MS_SQL_NAMEDPIPE");
	insert(L"ATTm591199", L"MS_SQL_MULTIPROTOCOL");
	insert(L"ATTm591200", L"MS_SQL_SPX");
	insert(L"ATTm591201", L"MS_SQL_TCPIP");
	insert(L"ATTm591202", L"MS_SQL_APPLETALK");
	insert(L"ATTm591203", L"MS_SQL_VINES");
	insert(L"ATTq591204", L"MS_SQL_STATUS");
	insert(L"ATTm591205", L"MS_SQL_LASTUPDATEDDATE");
	insert(L"ATTm591206", L"MS_SQL_INFORMATIONURL");
	insert(L"ATTm591207", L"MS_SQL_CONNECTIONURL");
	insert(L"ATTm591208", L"MS_SQL_PUBLICATIONURL");
	insert(L"ATTm591209", L"MS_SQL_GPSLATITUDE");
	insert(L"ATTm591210", L"MS_SQL_GPSLONGITUDE");
	insert(L"ATTm591211", L"MS_SQL_GPSHEIGHT");
	insert(L"ATTm591212", L"MS_SQL_VERSION");
	insert(L"ATTm591213", L"MS_SQL_LANGUAGE");
	insert(L"ATTm591214", L"MS_SQL_DESCRIPTION");
	insert(L"ATTm591215", L"MS_SQL_TYPE");
	insert(L"ATTi591216", L"MS_SQL_INFORMATIONDIRECTORY");
	insert(L"ATTm591217", L"MS_SQL_DATABASE");
	insert(L"ATTi591218", L"MS_SQL_ALLOWANONYMOUSSUBSCRIPTION");
	insert(L"ATTm591219", L"MS_SQL_ALIAS");
	insert(L"ATTq591220", L"MS_SQL_SIZE");
	insert(L"ATTm591221", L"MS_SQL_CREATIONDATE");
	insert(L"ATTm591222", L"MS_SQL_LASTBACKUPDATE");
	insert(L"ATTm591223", L"MS_SQL_LASTDIAGNOSTICDATE");
	insert(L"ATTm591224", L"MS_SQL_APPLICATIONS");
	insert(L"ATTm591225", L"MS_SQL_KEYWORDS");
	insert(L"ATTm591226", L"MS_SQL_PUBLISHER");
	insert(L"ATTi591227", L"MS_SQL_ALLOWKNOWNPULLSUBSCRIPTION");
	insert(L"ATTi591228", L"MS_SQL_ALLOWIMMEDIATEUPDATINGSUBSCRIPTION");
	insert(L"ATTi591229", L"MS_SQL_ALLOWQUEUEDUPDATINGSUBSCRIPTION");
	insert(L"ATTi591230", L"MS_SQL_ALLOWSNAPSHOTFILESFTPDOWNLOADING");
	insert(L"ATTi591231", L"MS_SQL_THIRDPARTY");
	insert(L"ATTk591524", L"MS_TAPI_CONFERENCE_BLOB");
	insert(L"ATTm591525", L"MS_TAPI_IP_ADDRESS");
	insert(L"ATTm591523", L"MS_TAPI_PROTOCOL_ID");
	insert(L"ATTm591522", L"MS_TAPI_UNIQUE_IDENTIFIER");
	insert(L"ATTm591447", L"MS_WMI_AUTHOR");
	insert(L"ATTm591448", L"MS_WMI_CHANGEDATE");
	insert(L"ATTm591500", L"MS_WMI_CLASS");
	insert(L"ATTm591449", L"MS_WMI_CLASSDEFINITION");
	insert(L"ATTm591450", L"MS_WMI_CREATIONDATE");
	insert(L"ATTj591501", L"MS_WMI_GENUS");
	insert(L"ATTm591451", L"MS_WMI_ID");
	insert(L"ATTj591452", L"MS_WMI_INTDEFAULT");
	insert(L"ATTj591502", L"MS_WMI_INTFLAGS1");
	insert(L"ATTj591503", L"MS_WMI_INTFLAGS2");
	insert(L"ATTj591504", L"MS_WMI_INTFLAGS3");
	insert(L"ATTj591505", L"MS_WMI_INTFLAGS4");
	insert(L"ATTj591453", L"MS_WMI_INTMAX");
	insert(L"ATTj591454", L"MS_WMI_INTMIN");
	insert(L"ATTj591455", L"MS_WMI_INTVALIDVALUES");
	insert(L"ATTq591456", L"MS_WMI_INT8DEFAULT");
	insert(L"ATTq591457", L"MS_WMI_INT8MAX");
	insert(L"ATTq591458", L"MS_WMI_INT8MIN");
	insert(L"ATTq591459", L"MS_WMI_INT8VALIDVALUES");
	insert(L"ATTm591462", L"MS_WMI_MOF");
	insert(L"ATTm591463", L"MS_WMI_NAME");
	insert(L"ATTm591464", L"MS_WMI_NORMALIZEDCLASS");
	insert(L"ATTm591506", L"MS_WMI_PARM1");
	insert(L"ATTm591507", L"MS_WMI_PARM2");
	insert(L"ATTm591508", L"MS_WMI_PARM3");
	insert(L"ATTm591509", L"MS_WMI_PARM4");
	insert(L"ATTm591465", L"MS_WMI_PROPERTYNAME");
	insert(L"ATTm591466", L"MS_WMI_QUERY");
	insert(L"ATTm591467", L"MS_WMI_QUERYLANGUAGE");
	insert(L"ATTm591510", L"MS_WMI_SCOPEGUID");
	insert(L"ATTm591468", L"MS_WMI_SOURCEORGANIZATION");
	insert(L"ATTm591460", L"MS_WMI_STRINGDEFAULT");
	insert(L"ATTm591461", L"MS_WMI_STRINGVALIDVALUES");
	insert(L"ATTm591469", L"MS_WMI_TARGETCLASS");
	insert(L"ATTm591470", L"MS_WMI_TARGETNAMESPACE");
	insert(L"ATTk591471", L"MS_WMI_TARGETOBJECT");
	insert(L"ATTm591472", L"MS_WMI_TARGETPATH");
	insert(L"ATTm591473", L"MS_WMI_TARGETTYPE");
	insert(L"ATTf590540", L"MSCOPE_ID");
	insert(L"ATTm590495", L"MSI_FILE_LIST");
	insert(L"ATTk590638", L"MSI_SCRIPT");
	insert(L"ATTm590669", L"MSI_SCRIPT_NAME");
	insert(L"ATTm589839", L"MSI_SCRIPT_PATH");
	insert(L"ATTj590670", L"MSI_SCRIPT_SIZE");
	insert(L"ATTi590747", L"MSMQ_AUTHENTICATE");
	insert(L"ATTj590744", L"MSMQ_BASE_PRIORITY");
	insert(L"ATTe590757", L"MSMQ_COMPUTER_TYPE");
	insert(L"ATTm591241", L"MSMQ_COMPUTER_TYPE_EX");
	insert(L"ATTj590770", L"MSMQ_COST");
	insert(L"ATTe590764", L"MSMQ_CSP_NAME");
	insert(L"ATTi591063", L"MSMQ_DEPENDENT_CLIENT_SERVICE");
	insert(L"ATTi591050", L"MSMQ_DEPENDENT_CLIENT_SERVICES");
	insert(L"ATTk590772", L"MSMQ_DIGESTS");
	insert(L"ATTk590790", L"MSMQ_DIGESTS_MIG");
	insert(L"ATTi591062", L"MSMQ_DS_SERVICE");
	insert(L"ATTi591052", L"MSMQ_DS_SERVICES");
	insert(L"ATTk590760", L"MSMQ_ENCRYPT_KEY");
	insert(L"ATTi590758", L"MSMQ_FOREIGN");
	insert(L"ATTb590753", L"MSMQ_IN_ROUTING_SERVERS");
	insert(L"ATTj591132", L"MSMQ_INTERVAL1");
	insert(L"ATTj591133", L"MSMQ_INTERVAL2");
	insert(L"ATTi590742", L"MSMQ_JOURNAL");
	insert(L"ATTj590745", L"MSMQ_JOURNAL_QUOTA");
	insert(L"ATTe590746", L"MSMQ_LABEL");
	insert(L"ATTm591239", L"MSMQ_LABEL_EX");
	insert(L"ATTj590765", L"MSMQ_LONG_LIVED");
	insert(L"ATTi590776", L"MSMQ_MIGRATED");
	insert(L"ATTm591538", L"MSMQ_MULTICAST_ADDRESS");
	insert(L"ATTi590763", L"MSMQ_NAME_STYLE");
	insert(L"ATTj590788", L"MSMQ_NT4_FLAGS");
	insert(L"ATTj590784", L"MSMQ_NT4_STUB");
	insert(L"ATTj590759", L"MSMQ_OS_TYPE");
	insert(L"ATTb590752", L"MSMQ_OUT_ROUTING_SERVERS");
	insert(L"ATTk590749", L"MSMQ_OWNER_ID");
	insert(L"ATTb591049", L"MSMQ_PREV_SITE_GATES");
	insert(L"ATTj590748", L"MSMQ_PRIVACY_LEVEL");
	insert(L"ATTk590775", L"MSMQ_QM_ID");
	insert(L"ATTj590787", L"MSMQ_QUEUE_JOURNAL_QUOTA");
	insert(L"ATTm591067", L"MSMQ_QUEUE_NAME_EXT");
	insert(L"ATTj590786", L"MSMQ_QUEUE_QUOTA");
	insert(L"ATTk590741", L"MSMQ_QUEUE_TYPE");
	insert(L"ATTj590743", L"MSMQ_QUOTA");
	insert(L"ATTm591519", L"MSMQ_RECIPIENT_FORMATNAME");
	insert(L"ATTi591061", L"MSMQ_ROUTING_SERVICE");
	insert(L"ATTi591051", L"MSMQ_ROUTING_SERVICES");
	insert(L"ATTi591537", L"MSMQ_SECURED_SOURCE");
	insert(L"ATTj590754", L"MSMQ_SERVICE_TYPE");
	insert(L"ATTj590774", L"MSMQ_SERVICES");
	insert(L"ATTk590771", L"MSMQ_SIGN_CERTIFICATES");
	insert(L"ATTk590791", L"MSMQ_SIGN_CERTIFICATES_MIG");
	insert(L"ATTk590761", L"MSMQ_SIGN_KEY");
	insert(L"ATTb590767", L"MSMQ_SITE_1");
	insert(L"ATTb590768", L"MSMQ_SITE_2");
	insert(L"ATTi590785", L"MSMQ_SITE_FOREIGN");
	insert(L"ATTb590769", L"MSMQ_SITE_GATES");
	insert(L"ATTb591134", L"MSMQ_SITE_GATES_MIG");
	insert(L"ATTk590777", L"MSMQ_SITE_ID");
	insert(L"ATTe590789", L"MSMQ_SITE_NAME");
	insert(L"ATTm591240", L"MSMQ_SITE_NAME_EX");
	insert(L"ATTk590751", L"MSMQ_SITES");
	insert(L"ATTi590750", L"MSMQ_TRANSACTIONAL");
	insert(L"ATTk591161", L"MSMQ_USER_SID");
	insert(L"ATTj590766", L"MSMQ_VERSION");
	insert(L"ATTi590943", L"MSNPALLOWDIALIN");
	insert(L"ATTf590947", L"MSNPCALLEDSTATIONID");
	insert(L"ATTf590948", L"MSNPCALLINGSTATIONID");
	insert(L"ATTf590954", L"MSNPSAVEDCALLINGSTATIONID");
	insert(L"ATTf590969", L"MSRADIUSCALLBACKNUMBER");
	insert(L"ATTj590977", L"MSRADIUSFRAMEDIPADDRESS");
	insert(L"ATTf590982", L"MSRADIUSFRAMEDROUTE");
	insert(L"ATTj590995", L"MSRADIUSSERVICETYPE");
	insert(L"ATTf591013", L"MSRASSAVEDCALLBACKNUMBER");
	insert(L"ATTj591014", L"MSRASSAVEDFRAMEDIPADDRESS");
	insert(L"ATTf591015", L"MSRASSAVEDFRAMEDROUTE");
	insert(L"ATTc131096", L"MUST_CONTAIN");
	insert(L"ATTj590577", L"NAME_SERVICE_FLAGS");
	insert(L"ATTb131088", L"NC_NAME");
	insert(L"ATTm589911", L"NETBIOS_NAME");
	insert(L"ATTi590673", L"NETBOOT_ALLOW_NEW_CLIENTS");
	insert(L"ATTi590678", L"NETBOOT_ANSWER_ONLY_VALID_CLIENTS");
	insert(L"ATTi590677", L"NETBOOT_ANSWER_REQUESTS");
	insert(L"ATTj590676", L"NETBOOT_CURRENT_CLIENT_COUNT");
	insert(L"ATTk590183", L"NETBOOT_GUID");
	insert(L"ATTm590182", L"NETBOOT_INITIALIZATION");
	insert(L"ATTm590681", L"NETBOOT_INTELLIMIRROR_OSES");
	insert(L"ATTi590674", L"NETBOOT_LIMIT_CLIENTS");
	insert(L"ATTm590683", L"NETBOOT_LOCALLY_INSTALLED_OSES");
	insert(L"ATTm590185", L"NETBOOT_MACHINE_FILE_PATH");
	insert(L"ATTj590675", L"NETBOOT_MAX_CLIENTS");
	insert(L"ATTm591065", L"NETBOOT_MIRROR_DATA_FILE");
	insert(L"ATTm590679", L"NETBOOT_NEW_MACHINE_NAMING_POLICY");
	insert(L"ATTb590680", L"NETBOOT_NEW_MACHINE_OU");
	insert(L"ATTb590688", L"NETBOOT_SCP_BL");
	insert(L"ATTb590684", L"NETBOOT_SERVER");
	insert(L"ATTm591064", L"NETBOOT_SIF_FILE");
	insert(L"ATTm590682", L"NETBOOT_TOOLS");
	insert(L"ATTe131531", L"NETWORK_ADDRESS");
	insert(L"ATTb590038", L"NEXT_LEVEL_STORE");
	insert(L"ATTj589912", L"NEXT_RID");
	insert(L"ATTb590354", L"NON_SECURITY_MEMBER");
	insert(L"ATTb590355", L"NON_SECURITY_MEMBER_BL");
	insert(L"ATTb590127", L"NOTIFICATION_LIST");
	insert(L"ATTk589913", L"NT_GROUP_MEMBERS");
	insert(L"ATTj590181", L"NT_MIXED_DOMAIN");
	insert(L"ATTk589918", L"NT_PWD_HISTORY");
	insert(L"ATTp131353", L"NT_SECURITY_DESCRIPTOR");
	insert(L"ATTb49", L"OBJ_DIST_NAME");
	insert(L"ATTb590606", L"OBJECT_CATEGORY");
	insert(L"ATTc0", L"OBJECT_CLASS");
	insert(L"ATTj131442", L"OBJECT_CLASS_CATEGORY");
	insert(L"ATTm1572870", L"OBJECT_CLASSES");
	insert(L"ATTj590330", L"OBJECT_COUNT");
	insert(L"ATTk589826", L"OBJECT_GUID");
	insert(L"ATTr589970", L"OBJECT_SID");
	insert(L"ATTj131148", L"OBJECT_VERSION");
	insert(L"ATTm589975", L"OEM_INFORMATION");
	insert(L"ATTk131290", L"OM_OBJECT_CLASS");
	insert(L"ATTj131303", L"OM_SYNTAX");
	insert(L"ATTk590329", L"OMT_GUID");
	insert(L"ATTk590157", L"OMT_INDX_GUID");
	insert(L"ATTm590187", L"OPERATING_SYSTEM");
	insert(L"ATTm590239", L"OPERATING_SYSTEM_HOTFIX");
	insert(L"ATTm590189", L"OPERATING_SYSTEM_SERVICE_PACK");
	insert(L"ATTm590188", L"OPERATING_SYSTEM_VERSION");
	insert(L"ATTj589968", L"OPERATOR_COUNT");
	insert(L"ATTm590536", L"OPTION_DESCRIPTION");
	insert(L"ATTj590131", L"OPTIONS");
	insert(L"ATTf590537", L"OPTIONS_LOCATION");
	insert(L"ATTm10", L"ORGANIZATION_NAME");
	insert(L"ATTm11", L"ORGANIZATIONAL_UNIT_NAME");
	insert(L"ATTm1376301", L"ORGANIZATIONALSTATUS");
	insert(L"ATTk131517", L"ORIGINAL_DISPLAY_TABLE");
	insert(L"ATTk131286", L"ORIGINAL_DISPLAY_TABLE_MSDOS");
	insert(L"ATTm589915", L"OTHER_LOGIN_WORKSTATIONS");
	insert(L"ATTm590475", L"OTHER_MAILBOX");
	insert(L"ATTm1441826", L"OTHER_NAME");
	insert(L"ATTh591183", L"OTHER_WELL_KNOWN_OBJECTS");
	insert(L"ATTb32", L"OWNER");
	insert(L"ATTj590151", L"PACKAGE_FLAGS");
	insert(L"ATTm590150", L"PACKAGE_NAME");
	insert(L"ATTj590148", L"PACKAGE_TYPE");
	insert(L"ATTb590381", L"PARENT_CA");
	insert(L"ATTk590509", L"PARENT_CA_CERTIFICATE_CHAIN");
	insert(L"ATTk591048", L"PARENT_GUID");
	insert(L"ATTk590487", L"PARTIAL_ATTRIBUTE_DELETION_LIST");
	insert(L"ATTk590464", L"PARTIAL_ATTRIBUTE_SET");
	insert(L"ATTq590690", L"PEK_KEY_CHANGE_INTERVAL");
	insert(L"ATTk590689", L"PEK_LIST");
	insert(L"ATTk590517", L"PENDING_CA_CERTIFICATES");
	insert(L"ATTb590519", L"PENDING_PARENT_CA");
	insert(L"ATTk131397", L"PER_MSG_DIALOG_DISPLAY_TABLE");
	insert(L"ATTk131398", L"PER_RECIP_DIALOG_DISPLAY_TABLE");
	insert(L"ATTm131687", L"PERSONAL_TITLE");
	insert(L"ATTm590470", L"PHONE_FAX_OTHER");
	insert(L"ATTm131349", L"PHONE_HOME_OTHER");
	insert(L"ATTm1376276", L"PHONE_HOME_PRIMARY");
	insert(L"ATTm590546", L"PHONE_IP_OTHER");
	insert(L"ATTm590545", L"PHONE_IP_PRIMARY");
	insert(L"ATTm590473", L"PHONE_ISDN_PRIMARY");
	insert(L"ATTm590471", L"PHONE_MOBILE_OTHER");
	insert(L"ATTm1376297", L"PHONE_MOBILE_PRIMARY");
	insert(L"ATTm131090", L"PHONE_OFFICE_OTHER");
	insert(L"ATTm131190", L"PHONE_PAGER_OTHER");
	insert(L"ATTm1376298", L"PHONE_PAGER_PRIMARY");
	insert(L"ATTk1376263", L"PHOTO");
	insert(L"ATTm19", L"PHYSICAL_DELIVERY_OFFICE_NAME");
	insert(L"ATTb590338", L"PHYSICAL_LOCATION_OBJECT");
	insert(L"ATTk1441827", L"PICTURE");
	insert(L"ATTm591154", L"PKI_CRITICAL_EXTENSIONS");
	insert(L"ATTm591158", L"PKI_DEFAULT_CSPS");
	insert(L"ATTj591151", L"PKI_DEFAULT_KEY_SPEC");
	insert(L"ATTp591159", L"PKI_ENROLLMENT_ACCESS");
	insert(L"ATTk591155", L"PKI_EXPIRATION_PERIOD");
	insert(L"ATTm591157", L"PKI_EXTENDED_KEY_USAGE");
	insert(L"ATTk591152", L"PKI_KEY_USAGE");
	insert(L"ATTj591153", L"PKI_MAX_ISSUING_DEPTH");
	insert(L"ATTk591156", L"PKI_OVERLAP_PERIOD");
	insert(L"ATTk590030", L"PKT");
	insert(L"ATTk590029", L"PKT_GUID");
	insert(L"ATTj590457", L"POLICY_REPLICATION_FLAGS");
	insert(L"ATTm590052", L"PORT_NAME");
	insert(L"ATTc131080", L"POSS_SUPERIORS");
	insert(L"ATTc590739", L"POSSIBLE_INFERIORS");
	insert(L"ATTm18", L"POST_OFFICE_BOX");
	insert(L"ATTm16", L"POSTAL_ADDRESS");
	insert(L"ATTm17", L"POSTAL_CODE");
	insert(L"ATTj28", L"PREFERRED_DELIVERY_METHOD");
	insert(L"ATTm1441831", L"PREFERREDLANGUAGE");
	insert(L"ATTb589921", L"PREFERRED_OU");
	insert(L"ATTk590362", L"PREFIX_MAP");
	insert(L"ATTn29", L"PRESENTATION_ADDRESS");
	insert(L"ATTk590516", L"PREVIOUS_CA_CERTIFICATES");
	insert(L"ATTb590518", L"PREVIOUS_PARENT_CA");
	insert(L"ATTj589922", L"PRIMARY_GROUP_ID");
	insert(L"ATTj591236", L"PRIMARY_GROUP_TOKEN");
	insert(L"ATTj590071", L"PRINT_ATTRIBUTES");
	insert(L"ATTm590061", L"PRINT_BIN_NAMES");
	insert(L"ATTi590066", L"PRINT_COLLATE");
	insert(L"ATTi590067", L"PRINT_COLOR");
	insert(L"ATTi591135", L"PRINT_DUPLEX_SUPPORTED");
	insert(L"ATTj590058", L"PRINT_END_TIME");
	insert(L"ATTm590059", L"PRINT_FORM_NAME");
	insert(L"ATTi590099", L"PRINT_KEEP_PRINTED_JOBS");
	insert(L"ATTm590070", L"PRINT_LANGUAGE");
	insert(L"ATTm590112", L"PRINT_MAC_ADDRESS");
	insert(L"ATTj590065", L"PRINT_MAX_COPIES");
	insert(L"ATTj590062", L"PRINT_MAX_RESOLUTION_SUPPORTED");
	insert(L"ATTj590101", L"PRINT_MAX_X_EXTENT");
	insert(L"ATTj590102", L"PRINT_MAX_Y_EXTENT");
	insert(L"ATTm590113", L"PRINT_MEDIA_READY");
	insert(L"ATTm590123", L"PRINT_MEDIA_SUPPORTED");
	insert(L"ATTj590106", L"PRINT_MEMORY");
	insert(L"ATTj590103", L"PRINT_MIN_X_EXTENT");
	insert(L"ATTj590104", L"PRINT_MIN_Y_EXTENT");
	insert(L"ATTm590111", L"PRINT_NETWORK_ADDRESS");
	insert(L"ATTm590096", L"PRINT_NOTIFY");
	insert(L"ATTj590114", L"PRINT_NUMBER_UP");
	insert(L"ATTm590064", L"PRINT_ORIENTATIONS_SUPPORTED");
	insert(L"ATTm590095", L"PRINT_OWNER");
	insert(L"ATTj590455", L"PRINT_PAGES_PER_MINUTE");
	insert(L"ATTj590109", L"PRINT_RATE");
	insert(L"ATTm590110", L"PRINT_RATE_UNIT");
	insert(L"ATTm590054", L"PRINT_SEPARATOR_FILE");
	insert(L"ATTm590094", L"PRINT_SHARE_NAME");
	insert(L"ATTm590098", L"PRINT_SPOOLING");
	insert(L"ATTi590105", L"PRINT_STAPLING_SUPPORTED");
	insert(L"ATTj590057", L"PRINT_START_TIME");
	insert(L"ATTm590097", L"PRINT_STATUS");
	insert(L"ATTm590124", L"PRINTER_NAME");
	insert(L"ATTq589923", L"PRIOR_SET_TIME");
	insert(L"ATTk589924", L"PRIOR_VALUE");
	insert(L"ATTj590055", L"PRIORITY");
	insert(L"ATTk589925", L"PRIVATE_KEY");
	insert(L"ATTj590460", L"PRIVILEGE_ATTRIBUTES");
	insert(L"ATTm590458", L"PRIVILEGE_DISPLAY_NAME");
	insert(L"ATTb590461", L"PRIVILEGE_HOLDER");
	insert(L"ATTq590459", L"PRIVILEGE_VALUE");
	insert(L"ATTk590642", L"PRODUCT_CODE");
	insert(L"ATTm589963", L"PROFILE_PATH");
	insert(L"ATTh591073", L"PROXIED_OBJECT_NAME");
	insert(L"ATTm131282", L"PROXY_ADDRESSES");
	insert(L"ATTi131595", L"PROXY_GENERATION_ENABLED");
	insert(L"ATTq589927", L"PROXY_LIFETIME");
	insert(L"ATTk590244", L"PUBLIC_KEY_POLICY");
	insert(L"ATTm590710", L"PURPORTED_SEARCH");
	insert(L"ATTj589919", L"PWD_HISTORY_LENGTH");
	insert(L"ATTq589920", L"PWD_LAST_SET");
	insert(L"ATTj589917", L"PWD_PROPERTIES");
	insert(L"ATTj590282", L"QUALITY_OF_SERVICE");
	insert(L"ATTm591179", L"QUERY_FILTER");
	insert(L"ATTb590432", L"QUERY_POLICY_BL");
	insert(L"ATTb590431", L"QUERY_POLICY_OBJECT");
	insert(L"ATTm590504", L"QUERYPOINT");
	insert(L"ATTj131106", L"RANGE_LOWER");
	insert(L"ATTj131107", L"RANGE_UPPER");
	insert(L"ATTm589825", L"RDN");
	insert(L"ATTc131098", L"RDN_ID");
	insert(L"ATTk26", L"REGISTERED_ADDRESS");
	insert(L"ATTm589929", L"REMOTE_SERVER_NAME");
	insert(L"ATTm589931", L"REMOTE_SOURCE");
	insert(L"ATTj589932", L"REMOTE_SOURCE_TYPE");
	insert(L"ATTm590633", L"REMOTE_STORAGE_GUID");
	insert(L"ATTk589827", L"REPL_PROPERTY_META_DATA");
	insert(L"ATTj590501", L"REPL_TOPOLOGY_STAY_OF_EXECUTION");
	insert(L"ATTk589828", L"REPL_UPTODATE_VECTOR");
	insert(L"ATTm589933", L"REPLICA_SOURCE");
	insert(L"ATTb131508", L"REPORTS");
	insert(L"ATTj591160", L"REPL_INTERVAL");
	insert(L"ATTk131163", L"REPS_FROM");
	insert(L"ATTk131155", L"REPS_TO");
	insert(L"ATTk590145", L"REQUIRED_CATEGORIES");
	insert(L"ATTk590497", L"RETIRED_REPL_DSA_SIGNATURES");
	insert(L"ATTr591125", L"TOKEN_GROUPS");
	insert(L"ATTr591242", L"TOKEN_GROUPS_GLOBAL_AND_UNIVERSAL");
	insert(L"ATTr591127", L"TOKEN_GROUPS_NO_GC_ACCEPTABLE");
	insert(L"ATTj589969", L"REVISION");
	insert(L"ATTj589977", L"RID");
	insert(L"ATTq590195", L"RID_ALLOCATION_POOL");
	insert(L"ATTq590194", L"RID_AVAILABLE_POOL");
	insert(L"ATTb590192", L"RID_MANAGER_REFERENCE");
	insert(L"ATTj590198", L"RID_NEXT_RID");
	insert(L"ATTq590196", L"RID_PREVIOUS_ALLOCATION_POOL");
	insert(L"ATTb590493", L"RID_SET_REFERENCES");
	insert(L"ATTq590197", L"RID_USED_POOL");
	insert(L"ATTm590164", L"RIGHTS_GUID");
	insert(L"ATTb33", L"ROLE_OCCUPANT");
	insert(L"ATTm1376262", L"ROOMNUMBER");
	insert(L"ATTb590498", L"ROOT_TRUST");
	insert(L"ATTm590190", L"RPC_NS_ANNOTATION");
	insert(L"ATTm589937", L"RPC_NS_BINDINGS");
	insert(L"ATTm590191", L"RPC_NS_CODESET");
	insert(L"ATTj590578", L"RPC_NS_ENTRY_FLAGS");
	insert(L"ATTm589938", L"RPC_NS_GROUP");
	insert(L"ATTm589939", L"RPC_NS_INTERFACE_ID");
	insert(L"ATTm590136", L"RPC_NS_OBJECT_ID");
	insert(L"ATTj589941", L"RPC_NS_PRIORITY");
	insert(L"ATTm589942", L"RPC_NS_PROFILE_ENTRY");
	insert(L"ATTm590138", L"RPC_NS_TRANSFER_SYNTAX");
	insert(L"ATTm590045", L"SAM_ACCOUNT_NAME");
	insert(L"ATTj590126", L"SAM_ACCOUNT_TYPE");
	insert(L"ATTk590035", L"SCHEDULE");
	insert(L"ATTj589944", L"SCHEMA_FLAGS_EX");
	insert(L"ATTk589972", L"SCHEMA_ID_GUID");
	insert(L"ATTk591182", L"SCHEMA_INFO");
	insert(L"ATTl590305", L"SCHEMA_UPDATE");
	insert(L"ATTj131543", L"SCHEMA_VERSION");
	insert(L"ATTj591178", L"SCOPE_FLAGS");
	insert(L"ATTm589886", L"SCRIPT_PATH");
	insert(L"ATTj591128", L"SD_RIGHTS_EFFECTIVE");
	insert(L"ATTj131406", L"SEARCH_FLAGS");
	insert(L"ATTk14", L"SEARCH_GUIDE");
	insert(L"ATTb1376277", L"SECRETARY");
	insert(L"ATTr589945", L"SECURITY_IDENTIFIER");
	insert(L"ATTb34", L"SEE_ALSO");
	insert(L"ATTj590328", L"SEQ_NOTIFICATION");
	insert(L"ATTf5", L"SERIAL_NUMBER");
	insert(L"ATTm590047", L"SERVER_NAME");
	insert(L"ATTb590339", L"SERVER_REFERENCE");
	insert(L"ATTb590340", L"SERVER_REFERENCE_BL");
	insert(L"ATTj589981", L"SERVER_ROLE");
	insert(L"ATTj589978", L"SERVER_STATE");
	insert(L"ATTm590334", L"SERVICE_BINDING_INFORMATION");
	insert(L"ATTk589946", L"SERVICE_CLASS_ID");
	insert(L"ATTk589947", L"SERVICE_CLASS_INFO");
	insert(L"ATTm590333", L"SERVICE_CLASS_NAME");
	insert(L"ATTm590481", L"SERVICE_DNS_NAME");
	insert(L"ATTm590483", L"SERVICE_DNS_NAME_TYPE");
	insert(L"ATTk590023", L"SERVICE_INSTANCE_VERSION");
	insert(L"ATTm590595", L"SERVICE_PRINCIPAL_NAME");
	insert(L"ATTm590149", L"SETUP_COMMAND");
	insert(L"ATTm590439", L"SHELL_CONTEXT_MENU");
	insert(L"ATTm590387", L"SHELL_PROPERTY_PAGES");
	insert(L"ATTm591033", L"SHORT_SERVER_NAME");
	insert(L"ATTb590468", L"SHOW_IN_ADDRESS_BOOK");
	insert(L"ATTi131241", L"SHOW_IN_ADVANCED_VIEW_ONLY");
	insert(L"ATTr590433", L"SID_HISTORY");
	insert(L"ATTm590648", L"SIGNATURE_ALGORITHMS");
	insert(L"ATTk590186", L"SITE_GUID");
	insert(L"ATTb590646", L"SITE_LINK_LIST");
	insert(L"ATTb590645", L"SITE_LIST");
	insert(L"ATTb590336", L"SITE_OBJECT");
	insert(L"ATTb590337", L"SITE_OBJECT_BL");
	insert(L"ATTb590318", L"SITE_SERVER");
	insert(L"ATTm590610", L"SMTP_MAIL_ADDRESS");
	insert(L"ATTm591171", L"SPN_MAPPINGS");
	insert(L"ATTm8", L"STATE_OR_PROVINCE_NAME");
	insert(L"ATTm9", L"STREET_ADDRESS");
	insert(L"ATTc1572873", L"STRUCTURAL_OBJECT_CLASS");
	insert(L"ATTc131093", L"SUB_CLASS_OF");
	insert(L"ATTb131079", L"SUB_REFS");
	insert(L"ATTb1638410", L"SUBSCHEMASUBENTRY");
	insert(L"ATTm590535", L"SUPER_SCOPE_DESCRIPTION");
	insert(L"ATTf590534", L"SUPER_SCOPES");
	insert(L"ATTm590356", L"SUPERIOR_DNS_ROOT");
	insert(L"ATTk589949", L"SUPPLEMENTAL_CREDENTIALS");
	insert(L"ATTk30", L"SUPPORTED_APPLICATION_CONTEXT");
	insert(L"ATTm4", L"SURNAME");
	insert(L"ATTj590490", L"SYNC_ATTRIBUTES");
	insert(L"ATTb590489", L"SYNC_MEMBERSHIP");
	insert(L"ATTb590488", L"SYNC_WITH_OBJECT");
	insert(L"ATTr590491", L"SYNC_WITH_SID");
	insert(L"ATTc590022", L"SYSTEM_AUXILIARY_CLASS");
	insert(L"ATTj590199", L"SYSTEM_FLAGS");
	insert(L"ATTc590020", L"SYSTEM_MAY_CONTAIN");
	insert(L"ATTc590021", L"SYSTEM_MUST_CONTAIN");
	insert(L"ATTi589994", L"SYSTEM_ONLY");
	insert(L"ATTc590019", L"SYSTEM_POSS_SUPERIORS");
	insert(L"ATTm20", L"TELEPHONE_NUMBER");
	insert(L"ATTk22", L"TELETEX_TERMINAL_IDENTIFIER");
	insert(L"ATTk21", L"TELEX_NUMBER");
	insert(L"ATTm590472", L"TELEX_PRIMARY");
	insert(L"ATTb591170", L"TEMPLATE_ROOTS");
	insert(L"ATTk590709", L"TERMINAL_SERVER");
	insert(L"ATTm131203", L"TEXT_COUNTRY");
	insert(L"ATTm1376258", L"TEXT_ENCODED_OR_ADDRESS");
	insert(L"ATTq590327", L"TIME_REFRESH");
	insert(L"ATTq590326", L"TIME_VOL_CHANGE");
	insert(L"ATTm12", L"TITLE");
	insert(L"ATTj131126", L"TOMBSTONE_LIFETIME");
	insert(L"ATTc590719", L"TRANSPORT_ADDRESS_ATTRIBUTE");
	insert(L"ATTm590613", L"TRANSPORT_DLL_NAME");
	insert(L"ATTb590615", L"TRANSPORT_TYPE");
	insert(L"ATTi590630", L"TREAT_AS_LEAF");
	insert(L"ATTm590484", L"TREE_NAME");
	insert(L"ATTj590294", L"TRUST_ATTRIBUTES");
	insert(L"ATTk589953", L"TRUST_AUTH_INCOMING");
	insert(L"ATTk589959", L"TRUST_AUTH_OUTGOING");
	insert(L"ATTj589956", L"TRUST_DIRECTION");
	insert(L"ATTb590295", L"TRUST_PARENT");
	insert(L"ATTm589957", L"TRUST_PARTNER");
	insert(L"ATTj589958", L"TRUST_POSIX_OFFSET");
	insert(L"ATTj589960", L"TRUST_TYPE");
	insert(L"ATTj589979", L"UAS_COMPAT");
	insert(L"ATTm1376257", L"UID");
	insert(L"ATTm589961", L"UNC_NAME");
	insert(L"ATTk589914", L"UNICODE_PWD");
	insert(L"ATTm1376300", L"UNIQUEIDENTIFIER");
	insert(L"ATTb50", L"UNIQUEMEMBER");
	insert(L"ATTk590637", L"UPGRADE_PRODUCT_CODE");
	insert(L"ATTm590714", L"UPN_SUFFIXES");
	insert(L"ATTj589832", L"USER_ACCOUNT_CONTROL");
	insert(L"ATTk590469", L"USER_CERT");
	insert(L"ATTm589980", L"USER_COMMENT");
	insert(L"ATTm589962", L"USER_PARAMETERS");
	insert(L"ATTk35", L"USER_PASSWORD");
	insert(L"ATTm1376264", L"USERCLASS");
	insert(L"ATTk1442008", L"USERPKCS12");
	insert(L"ATTm590480", L"USER_PRINCIPAL_NAME");
	insert(L"ATTm590575", L"USER_SHARED_FOLDER");
	insert(L"ATTm590576", L"USER_SHARED_FOLDER_OTHER");
	insert(L"ATTk1310860", L"USER_SMIME_CERTIFICATE");
	insert(L"ATTm589910", L"USER_WORKSTATIONS");
	insert(L"ATTq131192", L"USN_CHANGED");
	insert(L"ATTq131091", L"USN_CREATED");
	insert(L"ATTq131339", L"USN_DSA_LAST_OBJ_REMOVED");
	insert(L"ATTj131541", L"USN_INTERSITE");
	insert(L"ATTq131193", L"USN_LAST_OBJ_REM");
	insert(L"ATTq590720", L"USN_SOURCE");
	insert(L"ATTj591180", L"VALID_ACCESSES");
	insert(L"ATTm590079", L"VENDOR");
	insert(L"ATTj589965", L"VERSION_NUMBER");
	insert(L"ATTj590152", L"VERSION_NUMBER_HI");
	insert(L"ATTj590153", L"VERSION_NUMBER_LO");
	insert(L"ATTk590160", L"VOL_TABLE_GUID");
	insert(L"ATTk590158", L"VOL_TABLE_IDX_GUID");
	insert(L"ATTj590331", L"VOLUME_COUNT");
	insert(L"ATTm590125", L"WBEM_PATH");
	insert(L"ATTh590442", L"WELL_KNOWN_OBJECTS");
	insert(L"ATTl131075", L"WHEN_CHANGED");
	insert(L"ATTl131074", L"WHEN_CREATED");
	insert(L"ATTk589966", L"WINSOCK_ADDRESSES");
	insert(L"ATTm131536", L"WWW_HOME_PAGE");
	insert(L"ATTm590573", L"WWW_PAGE_OTHER");
	insert(L"ATTg24", L"X121_ADDRESS");
	insert(L"ATTk45", L"X500UNIQUEIDENTIFIER");
	insert(L"ATTk36", L"X509_CERT");
}
