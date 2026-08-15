#include "materialdatabasedialog.h"

#include "periodic_table.h"

#include <QTreeView>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFontDatabase>
#include <QSplitter>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "json_defs_p.h"

NLOHMANN_JSON_SERIALIZE_ENUM(MaterialDatabaseDialog::composition_type_t,
                             { { MaterialDatabaseDialog::atomic, "atomic" },
                               { MaterialDatabaseDialog::mass, "mass" } })

// MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MaterialDatabaseDialog::material_datasheet, id, title,
//                                          density, composition_type, Z, X, comment)

// serialization of mcconfig struct
void from_json(const ojson &j, MaterialDatabaseDialog::material_datasheet &md)
{
    md.id = j["id"];
    md.title = j["title"];
    md.density = j["density"];
    md.composition_type = j["composition_type"];
    md.comment = j["comment"];
    md.source = j["source"];
    ojson Z = j["Z"];
    if (Z.is_array())
        Z.get_to(md.Z);
    else {
        md.Z.resize(1);
        Z.get_to(md.Z.at(0));
    }
    ojson X = j["X"];
    if (X.is_array())
        X.get_to(md.X);
    else {
        md.X.resize(1);
        X.get_to(md.X.at(0));
    }
}

class MyFilterModel : public QSortFilterProxyModel
{
public:
    MyFilterModel(QObject *obj) : QSortFilterProxyModel(obj) { }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        QModelIndex i = sourceModel()->index(sourceRow, 0, sourceParent);
        // match either the name  or tooltip(id) of material
        return sourceModel()->data(i).toString().contains(filterRegExp())
                || sourceModel()->data(i, Qt::ToolTipRole).toString().contains(filterRegExp());
    }
};

MaterialDatabaseDialog::MaterialDatabaseDialog(const QStringList &existingMaterialIds,
                                               QWidget *parent)
    : QDialog(parent), existingMaterialIds_(QSet<QString>(existingMaterialIds.begin(),
                                                          existingMaterialIds.end()))
{
    setWindowTitle("OpenTRIM - Select Target Material");
    setMinimumSize(800, 400);

    QFile loadFile(QStringLiteral(":/md/material_card.html"));
    loadFile.open(QIODevice::ReadOnly);
    material_card_tmpl = loadFile.readAll();

    loadMaterialsDb();

    filterLineEdit = new QLineEdit;
    filterLineEdit->setPlaceholderText("Filter materials...");
    filterLineEdit->setClearButtonEnabled(true);

    /* create left-side tree widget */
    materialsTreeView = new QTreeView;
    proxyModel = new MyFilterModel(this);
    proxyModel->setSourceModel(materials_db);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setRecursiveFilteringEnabled(true);
    materialsTreeView->setModel(proxyModel);
    materialsTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    materialsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QScrollArea *sa = new QScrollArea;
    sa->setFrameShape(QFrame::StyledPanel);
    sa->setFrameShadow(QFrame::Sunken);
    sa->setWidgetResizable(true);

    materialLabel = new QLabel;
    materialLabel->setStyleSheet(QStringLiteral("background-color: white;"));
    materialLabel->setWordWrap(true);
    materialLabel->setOpenExternalLinks(true);
    materialLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    // materialLabel->setTextFormat(Qt::MarkdownText);
    materialLabel->setTextFormat(Qt::RichText);
    materialLabel->setAlignment(Qt::AlignTop);
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    materialLabel->setFont(monoFont);

    sa->setWidget(materialLabel);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(filterLineEdit, &QLineEdit::textChanged, this, &MaterialDatabaseDialog::onFilterTextChanged);
    connect(materialsTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &MaterialDatabaseDialog::onMaterialSelected);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QWidget *leftPanel = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    leftPanel->setLayout(vbox);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(filterLineEdit);
    vbox->addWidget(materialsTreeView);

    QSplitter *splitter = new QSplitter;
    splitter->addWidget(leftPanel);
    splitter->addWidget(sa);
    splitter->setSizes({ 200, 600 });

    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->addWidget(splitter);
    containerLayout->addWidget(buttonBox);
}

material::material_desc_t MaterialDatabaseDialog::getSelectedMaterial() const
{
    material::material_desc_t md;

    if (selected_.id.empty())
        return md;

    md.id = selected_.id;
    md.density = selected_.density;
    for (int i = 0; i < int(selected_.Z.size()); ++i) {
        atom::parameters p;
        p.element.atomic_number = selected_.Z[i];
        p.element.symbol = periodic_table::at(selected_.Z[i]).symbol;
        p.element.atomic_mass = periodic_table::at(selected_.Z[i]).mass;
        p.X = selected_.X[i];
        md.composition.push_back(p);
    }

    return md;
}

void MaterialDatabaseDialog::loadMaterialsDb()
{
    // Note: The path should point to the resource file once added to the .qrc
    QFile file(":/data/materials.json");

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open material database.");
        return;
    }

    ojson doc;
    materials_db = new QStandardItemModel(0, 1, this);
    materials_db->setHeaderData(0, Qt::Horizontal, "Materials");
    QStandardItem *parentItem = materials_db->invisibleRootItem();
    try {
        doc = ojson::parse(file.readAll().toStdString(), nullptr, true, true);

        for (const auto &group : doc) {
            std::string title = group["title"];
            QStandardItem *g =
                    new QStandardItem(QIcon(":/assets/ionicons/folder-outline.svg"), title.c_str());
            g->setToolTip(title.c_str());
            parentItem->appendRow(g);

            for (const auto &m : group["items"]) {
                material_datasheet md;
                m.get_to(md);
                QStandardItem *mi = new QStandardItem(
                        QIcon(":/assets/ionicons/document-text-outline.svg"), md.title.c_str());
                mi->setData(QVariant::fromValue(md));
                mi->setToolTip(md.id.c_str());
                g->appendRow(mi);
            }
        }
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error parsing materials db", e.what());
        return;
    }
}

void MaterialDatabaseDialog::onMaterialSelected(const QModelIndex &selected,
                                                const QModelIndex &deselected)
{
    QModelIndex sourceSelected = proxyModel->mapToSource(selected);
    QStandardItem *item =
            sourceSelected.isValid() ? materials_db->itemFromIndex(sourceSelected) : nullptr;
    materialLabel->clear();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    selected_ = { "", "", 0.f, atomic, {}, {}, "", "" };

    if (!item) {
        return;
    }

    QVariant V = item->data();
    if (!V.isValid())
        return;

    const material_datasheet &md = V.value<material_datasheet>();

    // calc normalized atomic frac
    std::vector<float> x(md.X);
    float sum(0.f);
    for (const auto &xi : x)
        sum += xi;
    for (auto &xi : x)
        xi /= sum;
    if (md.composition_type == mass) {
        for (int i = 0; i < int(md.Z.size()); ++i) {
            const auto &e = periodic_table::at(md.Z[i]);
            x[i] /= e.mass;
        }
        sum = 0.f;
        for (const auto &xi : x)
            sum += xi;
        for (auto &xi : x)
            xi /= sum;
    }

    selected_ = md;
    selected_.composition_type = atomic;
    selected_.X = x;

    // md
    // QString tbl;
    // for (int i = 0; i < int(md.Z.size()); ++i) {
    //     const auto &e = periodic_table::at(md.Z[i]);
    //     tbl += QString("| %1(Z=%2) | %3 |\n")
    //                    .arg(e.symbol.c_str())
    //                    .arg(e.Z)
    //                    .arg(100 * x[i], 6, 'f', 3);
    // }
    // QString comment(md.comment.c_str());
    // comment.replace(QChar('\n'), " <br />");
    // QString S = QString(material_card_tmpl)
    //                     .arg(md.title.c_str())
    //                     .arg(md.id.c_str())
    //                     .arg(md.density)
    //                     .arg(tbl)
    //                     .arg(comment);

    // html
    QString tbl;
    for (int i = 0; i < int(md.Z.size()); ++i) {
        const auto &e = periodic_table::at(md.Z[i]);
        tbl += QString("<tr><td style=\"border: 1px solid #999;\">%1(Z=%2)</td><td "
                       "style=\"border: 1px solid #999;\">%3</td></tr>\n")
                       .arg(e.symbol.c_str())
                       .arg(e.Z)
                       .arg(100 * x[i], 6, 'f', 3);
    }

    QString comment(md.comment.c_str());
    comment.replace(QChar('\n'), "<br/>");

    QString S = QString(material_card_tmpl)
                        .arg(md.title.c_str())
                        .arg(md.id.c_str())
                        .arg(md.density)
                        .arg(tbl)
                        .arg(comment)
                        .arg(md.source.c_str());

    materialLabel->setText(S);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void MaterialDatabaseDialog::onFilterTextChanged(const QString &text)
{
    proxyModel->setFilterWildcard(text);
    // clearDetails();
    // buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}
